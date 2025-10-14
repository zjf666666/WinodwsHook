#include "pch.h"
#include "NamedPipeServer.h"

#include <sddl.h>

#include "../SecurityCore/Logger.h"
#include "../SecurityCore/HandleWrapper.h"
#include "../SecurityCore/StringUtils.h"
#include "../SecurityCore/MemoryUtils.h"

#include "ClientSession.h"

#define MAX_PIPE_NAME_LENGTH    256  // 最长命名管道名称
#define BUFFER_SIZE             4096 // 设置命名管道接收/发送消息大小

#define CONNECT_SUCCESS             1    // 连接成功
#define CONNECT_FAILED              2    // 连接失败，执行下一个即可
#define CONNECT_WAIT_EXCEPTION      3    // 等待连接异常，需要退出循环

NamedPipeServer::NamedPipeServer() : m_bIsInit(false), m_bIsRunning(false)
{

}

NamedPipeServer::~NamedPipeServer()
{

}

bool NamedPipeServer::Initialize(const std::wstring& pipeName, const std::wstring& sddl)
{
    Logger::GetInstance().Info(L"Init named pipe server.");
    if (pipeName.empty())
    {
        Logger::GetInstance().Error(L"Pipe name is empty!");
        return false;
    }

    // 不允许反复初始化
    if (m_bIsInit)
    {
        Logger::GetInstance().Info(L"Pipe is already init.");
        return true;
    }

    m_wstrPipName = L"\\\\.\\pipe\\" + pipeName;

    if (m_wstrPipName.size() > MAX_PIPE_NAME_LENGTH)
    {
        Logger::GetInstance().Info(L"Pipe name is too long! pipe name = %s", pipeName.c_str());
        m_wstrPipName = L"";
        m_bIsInit = false;
        return false;
    }

    m_wstrSDDL = sddl;
    m_bIsInit = true;
    if (nullptr != m_sa.lpSecurityDescriptor)
    {
        LocalFree(m_sa.lpSecurityDescriptor);
    }
    m_sa = {};
    return true;
}

bool NamedPipeServer::Start()
{
    Logger::GetInstance().Info(L"Start named pipe server.");
    if (false == m_bIsInit)
    {
        Logger::GetInstance().Error(L"Pipe not init!");
        return false;
    }

    if (m_bIsRunning)
    {
        Logger::GetInstance().Error(L"Pipe is already running!");
        return true;
    }

    if (false == GenerateSecurityAttributes())
    {
        Logger::GetInstance().Error(L"SDDL %s is invalid!", m_wstrSDDL.c_str());
        return false;
    }

    m_thdAcceptLoop = std::thread(&NamedPipeServer::AcceptLoop, this);
    m_bIsRunning = true;
    return true;
}

void NamedPipeServer::Stop()
{
    Logger::GetInstance().Info(L"Stop named pipe server.");
}

void NamedPipeServer::SetCommandRegistry(std::shared_ptr<CommandRegistry> cmdReg)
{
    m_cmdRegistry = cmdReg;
}

bool NamedPipeServer::GenerateSecurityAttributes()
{
    PSECURITY_DESCRIPTOR pd = nullptr;
    m_sa.bInheritHandle = FALSE; // 不继承
    m_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    if (m_wstrSDDL.empty())
    {
        return true;
    }
    // 构建安全描述符
    // windowsAPI 不支持C++的异常捕获机制，使用C风格异常捕获
    // 这种形式内部无法展开C++风格的对象，如string、vector、自己定义的类等
    __try
    {
        // ConvertStringSecurityDescriptorToSecurityDescriptorW可能会抛出异常
        if (FALSE == ConvertStringSecurityDescriptorToSecurityDescriptorW(
            m_wstrSDDL.c_str(),
            SDDL_REVISION_1,
            &pd,
            nullptr))
        {
            return false;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }

    m_sa.lpSecurityDescriptor = pd;
    return true;
}

void NamedPipeServer::AcceptLoop()
{
    while (m_bIsRunning)
    {
        // 创建命名管道句柄
        HandleWrapper<> hPipe(CreateNamedPipeW(
            m_wstrPipName.c_str(),
            PIPE_ACCESS_DUPLEX, // 全双工管道，不支持异步IO 从业务上看，不会有很多客户端连接
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // 消息流而不是字节流，传输的是完整协议消息
            PIPE_UNLIMITED_INSTANCES, // 支持多个客户端连接
            BUFFER_SIZE,
            BUFFER_SIZE,
            0,
            &m_sa
        ));

        if (!hPipe.IsValid())
        {
            Logger::GetInstance().Error(L"Create named pipe failed! error = %d", GetLastError());
            continue;
        }

        // 等待客户端链接
        int nRes = WaitClientConnect(hPipe.Get());
        if (CONNECT_SUCCESS != nRes)
        {
            continue;
        }

        if (false == m_bIsRunning)
        {
            break;
        }

        // 连接成功后，将管道句柄的所有权转移到 ClientSession，避免本地 RAII 包装析构时提前关闭句柄
        // 注意：必须使用 Release/Detach，而不是 Get，否则本地 hPipe 在离开作用域时会 CloseHandle，导致后续 ReadFile/WriteFile 失败
        HANDLE hClientPipe = hPipe.Release();
        CreateClientSession(hClientPipe);

        // 清理会话
        ClearClientSession();
    }
}

int NamedPipeServer::WaitClientConnect(HANDLE hHandle)
{
    // 等待客户端链接
    bool bIsConnect = ConnectNamedPipe(hHandle, NULL);
    if (!bIsConnect)
    {
        DWORD dwError = GetLastError();

        // 返回已连接错误，认为是连接成功
        if (ERROR_PIPE_CONNECTED != dwError)
        {
            Logger::GetInstance().Error(L"Connect named pipe failed! error = %d", GetLastError());
            return CONNECT_FAILED;
        }
    }
    return CONNECT_SUCCESS;
}

void NamedPipeServer::CreateClientSession(HANDLE hHandle)
{
    auto session = std::make_shared<ClientSession>(hHandle, m_cmdRegistry);
    session->Start();
    std::weak_ptr<ClientSession> weakSession = session;
    {
        std::lock_guard<std::mutex> lock(m_mtxClientSession);
        m_vecSession.push_back(weakSession);
    }
}

void NamedPipeServer::ClearClientSession()
{
    std::lock_guard<std::mutex> lock(m_mtxClientSession);
    // 这里存在迭代器失效问题，erase之后，iter失效了，此时如果++iter会出现无法预估的错误
    // erase返回的iter就是擦除后指向的下一位迭代器，使用这个迭代器作为下一位
    for (auto iter = m_vecSession.begin(); iter != m_vecSession.end();)
    {
        if (nullptr == iter->lock())
        {
            iter = m_vecSession.erase(iter);
            continue;
        }
        ++iter;
    }
}
