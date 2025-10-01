#pragma once

#include "HookCommon.h"
#include "IHook.h"

class HookFactory
{
public:
    static IHook* CreateHook(HookType type);
};

