#pragma once

#include "HookCommon.h"
#include "IHookId.h"

class HookIdFactory
{
public:
    static IHookId* CreateHookId(HookType type);
};

