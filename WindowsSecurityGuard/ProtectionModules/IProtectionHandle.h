#pragma once

#include "../include/Common/Command.h"
#include "../include/Common/Param.h"

// ������������
class IProtectionHandle
{
public:
    IProtectionHandle() = default;
    virtual ~IProtectionHandle() = default;

    // ������
    virtual std::string Handle(CommandType type, Command cmd, const std::string& strJson) = 0;
};