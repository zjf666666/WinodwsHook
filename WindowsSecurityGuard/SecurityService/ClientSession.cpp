#include "pch.h"
#include "ClientSession.h"

#include "../SecurityCore/Logger.h"

#include "Message.h"
#include "CommandRegistry.h"

ClientSession::ClientSession(HANDLE pipe, std::shared_ptr<CommandRegistry> cmdRegistry) :
    m_hPipe(pipe), m_bIsRunning(false), m_cmdRegistry(cmdRegistry)
{

}

bool ClientSession::Start()
{
    if (m_bIsRunning)
    {
        Logger::GetInstance().Info(L"Client is running.");
        return true;
    }

    // TODO: 这个线程后面要改成线程池处理
    auto self = shared_from_this();
    m_thdLoop = std::thread(&ClientSession::RunLoop, self);

    m_bIsRunning = true;

    // TODO: 这里暂时保留返回值，后续如果没有false的可能就删了
    return true;
}

void ClientSession::Close()
{
    m_bIsRunning = false;

    // 关闭句柄（这会让阻塞的ReadFile返回）
    m_hPipe.Reset(nullptr);

    // 关闭线程
    if (m_thdLoop.joinable())
    {
        m_thdLoop.join();
    }
}

void ClientSession::RunLoop()
{
    WindowsSecurityGuard::Message request;
    while (m_bIsRunning)
    {
        // 1. 读取消息
        CommandType type;
        Command cmd;
        if (!ReadMessage(request, type, cmd, 10))
        {
            break; // 读取失败，退出循环
        }

        // 3. 分发消息到业务处理器
        
        HandlerFunc handler = m_cmdRegistry->GetHandler(type);
        if (nullptr == handler)
        {
            // TODO: 这里需要添加返回信息
            continue; // 获取处理函数失败，继续处理下一个请求 
        }

        WindowsSecurityGuard::Message response = *(handler(type, cmd, request));
        // 4. 发送响应
        //if (!WriteMessage(response, 10))
        //{
        //    break; // 写入失败，退出循环
        //}
    }

    // 线程退出时的清理工作
    m_bIsRunning = false;
}

bool ClientSession::ReadMessage(WindowsSecurityGuard::Message& message, CommandType& type, Command& cmd, DWORD timeoutMs)
{
    DWORD bytesRead = 0;
    if (FALSE == ReadFile(m_hPipe.Get(), &message.header, sizeof(message.header), &bytesRead, nullptr))
    {
        Logger::GetInstance().Error(L"ReadFile failed! error = %d", GetLastError());
        return false;
    }

    if (false == WindowsSecurityGuard::ProtocolUtils::ValidateHeader(message.header))
    {
        Logger::GetInstance().Error(L"Invaild header!");
        return false;
    }

    type = (CommandType)message.header.commandType;
    cmd = (Command)message.header.cmd;

    if (message.header.size > 0)
    {
        message.body.resize(message.header.size);  // 先分配内存
        if (FALSE == ReadFile(m_hPipe.Get(), message.body.data(), message.header.size, &bytesRead, nullptr))
        {
            Logger::GetInstance().Error(L"ReadFile failed! error = %d", GetLastError());
            return false;
        }
    }

    return true;
}

bool ClientSession::WriteMessage(const WindowsSecurityGuard::Message& message, DWORD timeoutMs)
{
    DWORD bytesWritten = 0;

    // 1. 写入消息头
    if (!WriteFile(m_hPipe.Get(), &message.header, sizeof(message.header), &bytesWritten, nullptr))
    {
        return false;
    }

    // 2. 写入消息体（如果有）
    if (message.header.size > 0)
    {
        if (!WriteFile(m_hPipe.Get(), message.body.data(), message.header.size, &bytesWritten, nullptr))
        {
            return false;
        }
    }

    return true;
}


