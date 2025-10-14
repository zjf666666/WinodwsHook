#pragma once

#include <functional>

#include "../include/common/Param.h"
#include "../include/common/Command.h"
#include "Message.h"

using HandlerFunc = std::function<const WindowsSecurityGuard::Message*(CommandType type, Command cmd, const WindowsSecurityGuard::Message& request)>;

class CommandRegistry
{
public:
    void RegisterHandler(CommandType type, HandlerFunc func)
    {
        handlers[type] = func;
    }

    HandlerFunc GetHandler(CommandType type) const
    {
        auto it = handlers.find(type);
        if (it != handlers.end())
        {
            return it->second;
        }
        return nullptr;
    }

private:
    std::unordered_map<CommandType, HandlerFunc> handlers;
};

