#include "pch.h"
#include "InlineHook.h"

InlineHook::InlineHook(const std::wstring& targetModule, const std::string& targetFunction, void* hookFunction) :
    m_bIsInstalled(false), m_wstrTargetModule(targetModule), m_strTargetFuncName(targetFunction), m_pHookFunction(hookFunction)
{
}

bool InlineHook::Install()
{
    return false;
}

bool InlineHook::Uninstall()
{
    return false;
}

bool InlineHook::IsInstalled() const
{
    return m_bIsInstalled;
}

const std::wstring& InlineHook::GetTargetModule() const
{
    return m_wstrTargetModule;
}

const std::string& InlineHook::GetTargetFunction() const
{
    return m_strTargetFuncName;
}

const std::wstring& InlineHook::GetHookType() const
{
    return L"InlineHook";
}

void* InlineHook::GetOriginalFunctionAddress() const
{
    return nullptr;
}
