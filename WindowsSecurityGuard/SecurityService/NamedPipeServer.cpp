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

    // 构建安全描述符
    PSECURITY_DESCRIPTOR pd = nullptr;
    __try
    {
        // ConvertStringSecurityDescriptorToSecurityDescriptorW可能会抛出异常
        if (!m_wstrSDDL.empty()) // 空sddl描述符就默认为无安全属性
        {
            if (FALSE == ConvertStringSecurityDescriptorToSecurityDescriptorW(
                m_wstrSDDL.c_str(),
                SDDL_REVISION_1,
                &pd,
                nullptr))
            {
                Logger::GetInstance().Error(L"Convert security descritor failed! error = %d", GetLastError());
                return false;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        Logger::GetInstance().Error(L"SDDL %s is invalid!", m_wstrSDDL.c_str());
        return false;
    }

    // 构建安全属性
    m_sa.bInheritHandle = FALSE; // 不继承
    m_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    m_sa.lpSecurityDescriptor = pd;

    // TODO: 后续需要引入线程池处理
    m_thdAcceptLoop = std::thread(&NamedPipeServer::AcceptLoop, this);
    m_bIsRunning = true;
    return true;
}

void NamedPipeServer::Stop()
{
    Logger::GetInstance().Info(L"Stop named pipe server.");
}

void NamedPipeServer::AcceptLoop()
{
    while (m_bIsRunning)
    {
        // 创建命名管道句柄
        HandleWrapper<> hPipe(CreatePipeInstance());
        if (false == hPipe.IsValid())
        {
            // TODO: 这里需要根据error添加更详细的错误处理
            continue;
        }

        // 创建异步等待事件
        OVERLAPPED overLapped = { 0 };
        overLapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

        // 等待客户端链接
        int nRes = WaitClientConnect(hPipe.Get(), &overLapped);
        if (CONNECT_WAIT_EXCEPTION == nRes || false == m_bIsRunning)
        {
            // 这里不用记日志，nRes的日志在函数内记录，m_bIsRunning停止时记录停止信息
            MemoryUtils::SafeCloseHandle(overLapped.hEvent);
            break;
        }

        // 连接失败，继续下一次等待
        if (CONNECT_FAILED == nRes)
        {
            MemoryUtils::SafeCloseHandle(overLapped.hEvent);
            continue;
        }

        // 释放event句柄
        MemoryUtils::SafeCloseHandle(overLapped.hEvent);

        // 连接成功 处理客户端会话信息
        CreateClientSession(hPipe.Get());

        // 清理会话
        ClearClientSession();
    }
}

HANDLE NamedPipeServer::CreatePipeInstance()
{
    HANDLE hPipe = CreateNamedPipeW(
        m_wstrPipName.c_str(),
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, // 全双工管道，支持异步I/O
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // 消息流而不是字节流，传输的是完整协议消息
        PIPE_UNLIMITED_INSTANCES, // 支持多个客户端连接
        BUFFER_SIZE,
        BUFFER_SIZE,
        0,
        &m_sa
    );

    if (INVALID_HANDLE_VALUE == hPipe)
    {
        Logger::GetInstance().Error(L"Create named pipe failed! error = %d", GetLastError());
        return INVALID_HANDLE_VALUE;
    }

    return hPipe;
}

int NamedPipeServer::WaitClientConnect(HANDLE hHandle, LPOVERLAPPED overLapped)
{
    // 等待客户端链接
    bool bIsConnect = ConnectNamedPipe(hHandle, overLapped);
    if (!bIsConnect)
    {
        DWORD dwError = GetLastError();
        if (ERROR_IO_PENDING == dwError)
        {
            DWORD dwWaitRes = WaitForSingleObject(overLapped->hEvent, INFINITE); // 无限等待
            // 无限等待时，如果返回错误，基本上都是系统级错误，无法恢复，继续循环也会一直失败
            if (WAIT_OBJECT_0 != dwWaitRes)
            {
                Logger::GetInstance().Error(L"WaitForSingleObject failed! error = %d", GetLastError());
                return CONNECT_WAIT_EXCEPTION;
            }
            return CONNECT_SUCCESS;
        }

        // 返回已连接错误，认为是连接成功
        if (ERROR_PIPE_CONNECTED == dwError)
        {
            return CONNECT_SUCCESS;
        }

        // 其他错误，记录日志，继续下一个
        Logger::GetInstance().Error(L"Connect named pipe failed! error = %d", GetLastError());
    }
    return CONNECT_FAILED;
}

void NamedPipeServer::CreateClientSession(HANDLE hHandle)
{
    auto session = std::make_shared<ClientSession>(hHandle, m_dispatcher);
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
