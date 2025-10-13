#pragma once

#include "../include/Common/Command.h"
#include "../include/Common/Param.h"

// 处理函数抽象类
class IProtectionHandle
{
public:
    IProtectionHandle() = default;
    virtual ~IProtectionHandle() = default;

    // 处理函数
    virtual std::string Handle(CommandType type, Command cmd, const std::string& strJson) = 0;
};