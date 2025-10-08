#pragma once

#include <functional>
#include <unordered_map>
#include <memory>

namespace WindowsSecurityGuard
{
    struct Message;
    enum class MessageType : uint32_t;
}

// 消息处理器函数类型
using MessageHandler = std::function<WindowsSecurityGuard::Message(const WindowsSecurityGuard::Message&)>;

class MessageDispatcher
{
public:
    WindowsSecurityGuard::Message Dispatch(const WindowsSecurityGuard::Message& request);

private:
    WindowsSecurityGuard::Message HandleProcessFileMointor(const WindowsSecurityGuard::Message& request);

private:
    std::unordered_map<WindowsSecurityGuard::MessageType, MessageHandler> m_mapHandler;
};

