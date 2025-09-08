#include "pch.h"
#include "Logger.h"
#include <Shlobj.h>

Logger::Logger() : hFile(INVALID_HANDLE_VALUE) {}

Logger::~Logger()
{
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
}

Logger& Logger::GetInstance()
{
    // C++11及之后版本静态变量的初始化是线程安全的，直接使用static创建一个logger对象并返回
    static Logger logger;
    return logger;
}

bool Logger::Initialize(const std::wstring& logPath, LogLevel minLevel)
{
    // 如果文件句柄已存在，重置句柄
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    // 创建目录
    size_t sizePos = logPath.find_last_of(L"\\/");
    if (sizePos != std::wstring::npos)
    {
        std::wstring wstrDir = logPath.substr(0, sizePos);
        DWORD dwRes = SHCreateDirectoryExW(NULL, wstrDir.c_str(), NULL);
        if (ERROR_SUCCESS != dwRes && ERROR_ALREADY_EXISTS != dwRes)
        {
            DWORD dwError = GetLastError();
            return false;
        }
    }

    // 文件不存在且创建失败则返回错误
    hFile = CreateFileW(logPath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        DWORD dwError = GetLastError();
        return false;
    }

    DWORD dwRes = SetFilePointer(hFile, 0, NULL, FILE_END); // 将文件写入指针置到文件末尾
    if (INVALID_SET_FILE_POINTER == dwRes) // 如果置到文件末尾失败，返回false，避免覆盖原日志内容
    {
        DWORD dwError = GetLastError();
        return false;
    }

    // 设置最小写入日志级别
    SetLogLevel(minLevel);
    return true;
}
