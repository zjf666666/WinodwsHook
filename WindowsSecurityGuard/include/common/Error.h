#pragma once

#include <string>
#include <unordered_map>

// 错误码的定义首先是帮助用户理解错误，其次是帮助开发精准定位问题
// 技术细节不应该过多暴露，比如注入作为Hook过程的一部分，用户是无感知的，如果是一个技术小白，无法理解这个错误如何解决
// 需要更具体的错误细节，比如是因为权限不足导致的还是其他的一些原因
enum class ErrorCode : uint32_t
{
    SUCCESS = 0,

    // TODO: 这里先写一下注入失败作为demo，得改成更具体的
    INJECT_FAILED = 0x1000
};

enum class Language : uint8_t
{
    CHINESE,
    ENGLISG
};

class ErrorManager
{
public:
    // 获取错误信息
    static std::string GetErrorMessage(ErrorCode error);
    static std::string GetErrorMessage(ErrorCode error, Language language);

    static void SetLanguage(Language language);

private:
    static const std::unordered_map<ErrorCode, std::unordered_map<Language, std::string>> umapError;
    static Language lan;
};