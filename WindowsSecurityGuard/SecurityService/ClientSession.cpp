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

    // TODO: ����̺߳���Ҫ�ĳ��̳߳ش���
    auto self = shared_from_this();
    m_thdLoop = std::thread(&ClientSession::RunLoop, self);

    m_bIsRunning = true;

    // TODO: ������ʱ��������ֵ���������û��false�Ŀ��ܾ�ɾ��
    return true;
}

void ClientSession::Close()
{
    m_bIsRunning = false;

    // �رվ���������������ReadFile���أ�
    m_hPipe.Reset(nullptr);

    // �ر��߳�
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
        // 1. ��ȡ��Ϣ
        CommandType type;
        Command cmd;
        if (!ReadMessage(request, type, cmd, 10))
        {
            break; // ��ȡʧ�ܣ��˳�ѭ��
        }

        // 3. �ַ���Ϣ��ҵ������
        
        HandlerFunc handler = m_cmdRegistry->GetHandler(type);
        if (nullptr == handler)
        {
            // TODO: ������Ҫ��ӷ�����Ϣ
            continue; // ��ȡ������ʧ�ܣ�����������һ������ 
        }

        WindowsSecurityGuard::Message response = *(handler(type, cmd, request));
        // 4. ������Ӧ
        //if (!WriteMessage(response, 10))
        //{
        //    break; // д��ʧ�ܣ��˳�ѭ��
        //}
    }

    // �߳��˳�ʱ��������
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
        message.body.resize(message.header.size);  // �ȷ����ڴ�
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

    // 1. д����Ϣͷ
    if (!WriteFile(m_hPipe.Get(), &message.header, sizeof(message.header), &bytesWritten, nullptr))
    {
        return false;
    }

    // 2. д����Ϣ�壨����У�
    if (message.header.size > 0)
    {
        if (!WriteFile(m_hPipe.Get(), message.body.data(), message.header.size, &bytesWritten, nullptr))
        {
            return false;
        }
    }

    return true;
}


