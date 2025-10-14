// SecurityService.cpp : 定义静态库的函数。
//

#include "pch.h"
#include "framework.h"
#include "SecurityService.h"
#include "Message.h"
#include "CommandRegistry.h"

typedef WindowsSecurityGuard::Message* (*RegisterModuleFunc)(CommandRegistry* registry);

bool SecurityService::Initialize()
{
    m_pipeServer = std::make_unique<NamedPipeServer>();
    m_pipeServer->Initialize(L"WindowsSecurityGuard", L"");

    // TODO: 这里的加载后续要改成动态扫描加载
    // 加载各个dll并获取type->handle的映射关系
    HMODULE hDll = LoadLibraryW(L"ProtectionModules.dll");
    if (nullptr == hDll)
    {
        return false;
    }

    // 获取导出函数
    RegisterModuleFunc func = (RegisterModuleFunc)GetProcAddress(hDll, "RegisterModule");
    if (nullptr == func)
    {
        return false;
    }
    m_cmdReg = std::make_shared<CommandRegistry>();
    func(m_cmdReg.get());
    m_pipeServer->SetCommandRegistry(m_cmdReg);
    return true;
}

bool SecurityService::Start()
{
    m_pipeServer->Start();
    return true;
}
