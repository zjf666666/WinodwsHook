#pragma once
#include "pch.h"

#include <string>
#include <memory>

#include "HookCommon.h"
#include "../include/common/Param.h"
#include "IHook.h"

struct HookIdInfo
{
    HookType type;                  // HOOK类型
    std::unique_ptr<IHook> hook;    // hook实例
    std::wstring wstrTargetModule;  // 目标模块名称
    std::string strTargetFunc;      // 目标函数名称
    std::string strArch;            // 架构
};

/* hookid生成抽象类 */
class IHookId
{
public:
    virtual ~IHookId() = default;
    virtual std::string GenerateKey(const Param& param, HookIdInfo* info) = 0;

protected:
    std::string ReplaceSpecChar(const std::string& str);
};