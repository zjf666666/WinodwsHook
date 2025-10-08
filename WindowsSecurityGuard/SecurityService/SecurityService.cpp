// SecurityService.cpp : 定义静态库的函数。
//

#include "pch.h"
#include "framework.h"
#include "SecurityService.h"

bool SecurityService::Initialize()
{
    m_pipeServer = std::make_unique<NamedPipeServer>();
    m_pipeServer->Initialize(L"WindowsSecurityGuard", L"");
    return true;
}

bool SecurityService::Start()
{
    m_pipeServer->Start();
    return true;
}
