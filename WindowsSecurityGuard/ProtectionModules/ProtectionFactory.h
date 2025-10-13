#pragma once

#include "IProtectionHandle.h"
#include "../include/common/Command.h"

class ProtectionFactory
{
public:
    static IProtectionHandle* Create(CommandType type);
};

