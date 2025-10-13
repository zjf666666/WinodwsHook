#include "pch.h"
#include "Error.h"

Language ErrorManager::lan = Language::ENGLISG;

const std::unordered_map<ErrorCode, std::unordered_map<Language, std::string>> ErrorManager::umapError =
{
    // 操作失败需要给出错误原因和解决方案，例如配置项范围为0-100，用户输入101，提示为：数值超出范围（允许范围：1-100），请重新输入
    {ErrorCode::SUCCESS, {{Language::CHINESE, "操作成功"}, {Language::ENGLISG, "success"}}},
    {ErrorCode::INJECT_FAILED, {{Language::CHINESE, "注入失败"}, {Language::ENGLISG, "inject failed"}}}
};


std::string ErrorManager::GetErrorMessage(ErrorCode error)
{
    return GetErrorMessage(error, lan);
}

std::string ErrorManager::GetErrorMessage(ErrorCode error, Language language)
{
    auto codeIter = umapError.find(error);
    if (codeIter == umapError.end())
    {
        return "";
    }

    auto langIter = codeIter->second.find(language);
    if (langIter == codeIter->second.end())
    {
        // 如果指定语言不存在，回退到英文
        langIter = codeIter->second.find(Language::ENGLISG);
        if (langIter == codeIter->second.end())
        {
            return "";
        }
    }

    return langIter->second;
}

void ErrorManager::SetLanguage(Language language)
{
    lan = language;
}
