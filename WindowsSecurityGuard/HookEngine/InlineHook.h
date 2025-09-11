#pragma once

#include "IHook.h"

class InlineHook : public IHook
{
public:
    InlineHook(const std::wstring& targetModule, const std::string& targetFunction, void* hookFunction);

    bool Install() override;

    bool Uninstall() override;

    bool IsInstalled() const override;

    const std::wstring& GetTargetModule() const override;

    const std::string& GetTargetFunction() const override;

    const std::wstring& GetHookType() const override;

protected:
    void* GetOriginalFunctionAddress() const override;

private:
    bool m_bIsInstalled;             // 是否已安装
    std::wstring m_wstrTargetModule; // 模块名称
    std::string m_strTargetFuncName;     // 函数名称
    void* m_pHookFunction;            // Hook 函数指针
    void* m_pOriginalFunction;        // 原始函数指针
    unsigned char m_originalBytes[16]; // 原始字节
};

