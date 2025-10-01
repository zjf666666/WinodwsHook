#include "pch.h"
#include "HookIdFactory.h"
#include "HookId.h"
IHookId* HookIdFactory::CreateHookId(HookType type)
{
    IHookId* hookId = nullptr;
    switch (type)
    {
    case HookType::InlineHook:
        hookId = new InlineHookId();
        break;
    case HookType::IATHook:
        hookId = new IATHookId();
        break;
    default:
        break;
    }
    return hookId;
}
