#include "pch.h"
#include "ProtectionModules.h"
#include "ProtectionFactory.h"

ProtectionModules& ProtectionModules::GetInstance()
{
    static ProtectionModules instance;
    return instance;
}

WindowsSecurityGuard::Message* ProtectionModules::Handle(CommandType type, Command cmd, const WindowsSecurityGuard::Message& request)
{
    // 转换请求为string 不要让更底层的类去依赖message 在上层将请求处理成json
    std::string strJson(request.body.begin(), request.body.end());
    IProtectionHandle* handle = ProtectionFactory::Create(type);
    std::string strRes = handle->Handle(type, cmd, strJson);

    // 初始化头部
    WindowsSecurityGuard::Message message(
        MessageType::RESPONSE,
        (CommandType)request.header.commandType,
        (Command)request.header.cmd,
        request.header.correlationId
    );

    // 写入消息体
    message.SetBody(strRes);

    return &message;
}
