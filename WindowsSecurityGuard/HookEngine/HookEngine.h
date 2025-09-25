#pragma once

#include "HookCommon.h"

class HookEngine
{
public:
    void StartHook(HookType type, const HookParam& param);

    void StopHook(HookType type, const HookParam& param);

private:
    
};

