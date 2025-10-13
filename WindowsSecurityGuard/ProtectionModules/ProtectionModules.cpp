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
    // ת������Ϊstring ��Ҫ�ø��ײ����ȥ����message ���ϲ㽫�������json
    std::string strJson(request.body.begin(), request.body.end());
    IProtectionHandle* handle = ProtectionFactory::Create(type);
    std::string strRes = handle->Handle(type, cmd, strJson);

    // ��ʼ��ͷ��
    WindowsSecurityGuard::Message message(
        MessageType::RESPONSE,
        (CommandType)request.header.commandType,
        (Command)request.header.cmd,
        request.header.correlationId
    );

    // д����Ϣ��
    message.SetBody(strRes);

    return &message;
}
