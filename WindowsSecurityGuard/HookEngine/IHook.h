#pragma once

#include <string>

#include "HookParam.h"

/* Hook 接口类，定义所有 Hook 实现的通用接口 */
class IHook
{
public:
    virtual ~IHook() = default; // 更符合现代C++风格习惯 等同于 ~IHook() {}

    // 初始化函数
    virtual bool Init(const HookParam& params) = 0;

    // 安装 Hook
    virtual bool Install() = 0;

    // 卸载 Hook
    virtual bool Uninstall() = 0;

    // 检查 Hook 是否已安装
    virtual bool IsInstalled() const = 0;
};

