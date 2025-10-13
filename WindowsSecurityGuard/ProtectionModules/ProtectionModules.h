#pragma once

/*
 * 业务接口类，单例类
 */

#include <unordered_map>
#include <vector>
#include "../include/common/Command.h"
#include "../SecurityService/Message.h"

enum class ProtectionType; // 前向声明

class ProtectionModules
{
public:
    static ProtectionModules& GetInstance();

    // 处理函数 
    static WindowsSecurityGuard::Message* Handle(CommandType type, Command cmd, const WindowsSecurityGuard::Message& request);
};

