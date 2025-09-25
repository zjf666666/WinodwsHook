#include "pch.h"
#include "HookEngine.h"
#include "HookFactory.h"

#include "../SecurityCore/Logger.h"

void HookEngine::StartHook(HookType type, const HookParam& param)
{
    // 根据类型创建hook类
    IHook* hook = HookFactory::CreateHook(type);
    if (nullptr == hook)
    {
        Logger::GetInstance().Error(L"Create hook failed!");
        return;
    }

    // 初始化hook类
    if (false == hook->Init(param))
    {
        Logger::GetInstance().Error(L"Init hook failed!");
        return;
    }

    // 安装hook类
    if (false == hook->Install())
    {
        Logger::GetInstance().Error(L"Install hook failed!");
        return;
    }
}
