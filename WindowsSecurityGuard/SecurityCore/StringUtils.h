#pragma once
#include <string>
#include <vector>

class StringUtils
{
public:
    // 宽字符转多字节
    static std::string WideToMultiByte(const std::wstring& wide);

    // 多字节转宽字符
    static std::wstring MultiByteToWide(const std::string& mb);

    // 格式化字符串，这里不可以传入引用参数，va_start明确指出，传入类型不可为引用
    static std::wstring FormatString(std::wstring format, ...);

    // 字符串分割
    static std::vector<std::wstring> SplitString(const std::wstring& str, wchar_t delimiter);

    // 路径规范化
    static std::wstring NormalizePath(const std::wstring& path);

    /**
     * @brief 在字符串中查找并替换所有指定的子字符串
     * @param [IN] str 原始字符串，需要进行替换操作的源字符串
     * @param [IN] from 需要被替换的子字符串
     * @param [IN] to 替换成的新子字符串
     * @return std::wstring 替换操作完成后的新字符串
     */
    static std::wstring ReplaceString(const std::wstring& str, const std::wstring& from, const std::wstring& to);

    // 字符串转小写
    static std::wstring ToLower(const std::wstring& str);

    // 字符串转大写
    static std::wstring ToUpper(const std::wstring& str);

public:
    // C风格字符串函数，如果需要跨模块调用，应使用C风格实现，避免跨模块传输中C++字符串出现的兼容性问题
    int WideToMultiByteC(const wchar_t* wide, int wideLen, char* buffer, int bufferSize);

private:
    /*
     * 以下代码为规范性代码，静态工具类应避免显式生成对象，使用::的形式进行调用
     * 删除拷贝构造函数及拷贝赋值函数避免友元函数或成员函数进行拷贝操作
     */
    StringUtils() {} // 私有构造函数，防止外部调用构造
    ~StringUtils() {} // 私有析构函数，防止外部调用析构
    StringUtils(const StringUtils&) = delete; // 删除拷贝构造函数
    StringUtils& operator=(const StringUtils&) = delete; // 删除拷贝赋值操作
};

