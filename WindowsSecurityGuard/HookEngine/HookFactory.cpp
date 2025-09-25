#include "pch.h"
#include "HookFactory.h"

#include "InlineHook.h"
#include "IATHook.h"

IHook* HookFactory::CreateHook(HookType type)
{
    IHook* hook = nullptr;
    switch (type)
    {
    case HookType::InlineHook:
        hook = new InlineHook();
        break;
    case HookType::IATHook:
        hook = new IATHook();
        break;
    default:
        break;
    }
    return hook;
}
