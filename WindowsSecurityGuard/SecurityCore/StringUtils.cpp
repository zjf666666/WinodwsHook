#include "pch.h"
#include "StringUtils.h"
#include <Windows.h>
#include <vector>
#include "Constants.h"

std::string StringUtils::WideToMultiByte(const std::wstring& wide)
{
    if (LENGTH_ZERO == wide.size())
    {
        return "";
    }

    int nLen = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), wide.size(), NULL, 0, NULL, NULL);
    if (LENGTH_ZERO == nLen) // 返回0说明执行错误
    {
        int nError = GetLastError();
        return "";
    }
    
    std::vector<char> vecBuffer(nLen + 1, 0);
    int nRes = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), wide.size(), vecBuffer.data(), nLen, NULL, NULL);
    if (LENGTH_ZERO == nRes) // 返回0说明执行错误
    {
        int nError = GetLastError();
        return "";
    }

    return std::string(vecBuffer.data(), nLen);
}

std::wstring StringUtils::MultiByteToWide(const std::string& mb)
{
    if (LENGTH_ZERO == mb.size())
    {
        return L"";
    }

    int nLen = MultiByteToWideChar(CP_ACP, 0, mb.c_str(), mb.size(), NULL, 0);
    if (LENGTH_ZERO == nLen) // 返回0说明执行错误
    {
        int nError = GetLastError();
        return L"";
    }

    std::vector<wchar_t> vecBuffer(nLen + 1, 0);
    int nRes = MultiByteToWideChar(CP_ACP, 0, mb.c_str(), mb.size(), vecBuffer.data(), nLen);
    if (LENGTH_ZERO == nRes) // 返回0说明执行错误
    {
        int nError = GetLastError();
        return L"";
    }

    return std::wstring(vecBuffer.data(), nLen);
}

std::wstring StringUtils::FormatString(std::wstring format, ...)
{
    // 声明并初始化可变参数列表
    va_list args;
    va_start(args, format);

    // 计算格式化后的字符串长度
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // 返回-1说明执行错误
    {
        return L"";
    }

    // args只能使用一次，每次使用后需要释放并重新start
    va_start(args, format);

    // 分配足够的缓冲区
    std::vector<wchar_t> vecBuffer(nLen + 1, 0);

    // 使用vswprintf_s执行格式化
    int nRes = vswprintf_s(vecBuffer.data(), nLen + 1, format.c_str(), args);

    // 清理可变参数列表
    va_end(args);
    if (LENGTH_MINUS_ONE == nRes)
    {
        return L"";
    }

    return std::wstring(vecBuffer.data(), nLen);
}

std::vector<std::wstring> StringUtils::SplitString(const std::wstring& str, wchar_t delimiter)
{
    std::vector<std::wstring> vecRes;
    if (LENGTH_ZERO == str.size())
    {
        return vecRes;
    }

    size_t sizeStartPos = 0; // 查找起点位置
    size_t sizeFoundPos = str.find(delimiter, sizeStartPos); // 查找结果位置

    while(sizeFoundPos != std::wstring::npos)
    {
        if (sizeFoundPos != sizeStartPos)
        {
            vecRes.push_back(str.substr(sizeStartPos, sizeFoundPos - sizeStartPos));
        }
        sizeStartPos = sizeFoundPos + 1;
        sizeFoundPos = str.find(delimiter, sizeStartPos);
    }

    if (sizeStartPos < str.size())
    {
        vecRes.push_back(str.substr(sizeStartPos));
    }

    return vecRes;
}

std::wstring StringUtils::NormalizePath(const std::wstring& path)
{
    if (LENGTH_ZERO == path.size())
    {
        return L"";
    }

    // 获取文件完整路径
    std::vector<WCHAR> vecBuffer(MAX_PATH, 0);
    DWORD dwRes = GetFullPathNameW(path.c_str(), MAX_PATH, vecBuffer.data(), NULL);
    if (ERROR_GET_FULL_PATH_NAME == dwRes)
    {
        int nError = GetLastError();
        return path;
    }

    if (dwRes > MAX_PATH)
    {
        vecBuffer.resize(dwRes + 1, 0);
        dwRes = GetFullPathNameW(path.c_str(), MAX_PATH, vecBuffer.data(), NULL);
        if (ERROR_GET_FULL_PATH_NAME == dwRes)
        {
            int nError = GetLastError();
            return path;
        }
    }

    std::wstring wstrPath = vecBuffer.data();
    int nPathSize = wstrPath.size();
    for (int i = 0; i < nPathSize; ++i)
    {
        if (L'/' == wstrPath[i])
        {
            wstrPath[i] = L'\\';
        }
    }
    if (wstrPath.size() > LENGTH_WINDOWS_ROOT_DIR && L'\\' == wstrPath[wstrPath.size() - 1])
    {
        wstrPath.pop_back();
    }

    return wstrPath;
}

std::wstring StringUtils::ReplaceString(const std::wstring& str, const std::wstring& from, const std::wstring& to)
{
    return std::wstring();
}

std::wstring StringUtils::ToLower(const std::wstring& str)
{
    return std::wstring();
}

std::wstring StringUtils::ToUpper(const std::wstring& str)
{
    return std::wstring();
}
