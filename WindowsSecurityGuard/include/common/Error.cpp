#include "pch.h"
#include "Error.h"

Language ErrorManager::lan = Language::ENGLISG;

const std::unordered_map<ErrorCode, std::unordered_map<Language, std::string>> ErrorManager::umapError =
{
    // ����ʧ����Ҫ��������ԭ��ͽ�����������������ΧΪ0-100���û�����101����ʾΪ����ֵ������Χ������Χ��1-100��������������
    {ErrorCode::SUCCESS, {{Language::CHINESE, "�����ɹ�"}, {Language::ENGLISG, "success"}}},
    {ErrorCode::INJECT_FAILED, {{Language::CHINESE, "ע��ʧ��"}, {Language::ENGLISG, "inject failed"}}}
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
        // ���ָ�����Բ����ڣ����˵�Ӣ��
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
