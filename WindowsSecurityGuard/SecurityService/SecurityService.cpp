// SecurityService.cpp : 定义静态库的函数。
//

#include "pch.h"
#include "framework.h"
#include "SecurityService.h"

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
    
    return true;
}

bool SecurityService::Start()
{
    m_pipeServer->Start();
    return true;
}
