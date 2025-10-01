#include "pch.h"
#include "HookId.h"

#include "../SecurityCore/Logger.h"
#include "../SecurityCore/StringUtils.h"

std::string InlineHookId::GenerateKey(const HookParam& param, HookIdInfo* info)
{
    auto strArch = param.Get<std::string>("common_architecture");
    auto targetFunc = param.Get<std::string>("inline_function_name");
    auto targetModule = param.Get<const wchar_t>("common_target_module");
    if (!strArch || !targetFunc || !targetModule)
    {
        Logger::GetInstance().Error(L"Get params failed!");
        return "";
    }

    std::string strKey = std::string("inline") + "|" + 
        ReplaceSpecChar(*strArch) + "|" + 
        ReplaceSpecChar(*targetFunc) + "|" + 
        ReplaceSpecChar(StringUtils::WideToMultiByte(targetModule));

    if (nullptr != info)
    {
        info->strArch = *strArch;
        info->strTargetFunc = *targetFunc;
        info->wstrTargetModule = targetModule;
    }
    return strKey;
}

std::string IATHookId::GenerateKey(const HookParam& param, HookIdInfo* info)
{
    auto strArch = param.Get<std::string>("common_architecture");
    auto targetFunc = param.Get<std::string>("inline_function_name");
    auto targetModule = param.Get<const wchar_t>("common_target_module");
    if (!strArch || !targetFunc || !targetModule)
    {
        Logger::GetInstance().Error(L"Get params failed!");
        return "";
    }

    std::string strKey = std::string("iat") + "|" +
        ReplaceSpecChar(*strArch) + "|" +
        ReplaceSpecChar(*targetFunc) + "|" +
        ReplaceSpecChar(StringUtils::WideToMultiByte(targetModule));

    if (nullptr != info)
    {
        info->strArch = *strArch;
        info->strTargetFunc = *targetFunc;
        info->wstrTargetModule = targetModule;
    }
    return strKey;
}
