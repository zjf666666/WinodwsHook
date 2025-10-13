#pragma once

#include "IHook.h"

#include <Windows.h>

// IATHOOK 不需要保留任何状态，所以不需要特定的构造函数
class IATHook : public IHook
{
public:
    // 初始化函数
    virtual bool Init(const Param& params) override;

    // 安装 Hook
    virtual bool Install() override;

    // 卸载 Hook
    virtual bool Uninstall() override;

    // 检查 Hook 是否已安装
    virtual bool IsInstalled() const override;

    void* GetOriginalFuncAddress() const;

private:
    PVOID m_pOriginalFunction; // 原函数地址

    bool m_bIsInstalled;
    void* m_pHookFunction;             // Hook函数地址
    std::wstring m_wstrTargetModule;   // 目标模块路径
    std::string m_strTargetFuncName;   // 目标函数名称
    bool m_bIs64Bit;
};

