#pragma once

#include "IHook.h"

class HookFactory
{
public:
    static IHook* CreateHook(HookType type);
};

