#pragma once

/*
 * ҵ��ӿ��࣬������
 */

#include <unordered_map>
#include <vector>
#include "../include/common/Command.h"
#include "../SecurityService/Message.h"

enum class ProtectionType; // ǰ������

class ProtectionModules
{
public:
    static ProtectionModules& GetInstance();

    // ������ 
    static WindowsSecurityGuard::Message* Handle(CommandType type, Command cmd, const WindowsSecurityGuard::Message& request);
};

