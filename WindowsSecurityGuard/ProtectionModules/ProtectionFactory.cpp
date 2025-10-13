#include "pch.h"
#include "ProtectionFactory.h"

#include "FileProtectionHandle.h"

IProtectionHandle* ProtectionFactory::Create(CommandType type)
{
    switch (type)
    {
    case CommandType::OPERATE_FILE:
        return new FileProtectionHandle();
        break;
    case CommandType::OPERATE_PROCESS:
        break;
    case CommandType::OPERATE_NETWORK:
        break;
    case CommandType::EVENT_FILE:
        break;
    case CommandType::EVENT_PROCESS:
        break;
    case CommandType::EVENT_NETWORK:
        break;
    default:
        break;
    }
    return nullptr;
}
