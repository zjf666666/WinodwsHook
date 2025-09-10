#include "pch.h"
#include "Logger.h"

// windows库
#include <Shlobj.h>

// 其他依赖库
#include "StringUtils.h"
#include "Constants.h"
#include "MemoryUtils.h"

Logger::Logger() : m_hFile(INVALID_HANDLE_VALUE), m_logLevel(LogLevel::Info) {}

Logger::~Logger()
{
    MemoryUtils::SafeCloseHandle(m_hFile, INVALID_HANDLE_VALUE);
}

Logger& Logger::GetInstance()
{
    // C++11及之后版本静态变量的初始化是线程安全的，直接使用static创建一个logger对象并返回
    static Logger logger;
    return logger;
}

bool Logger::Initialize(const std::wstring& logPath, LogLevel minLevel)
{
    // 重置句柄
    FreeHandle();

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
    m_hFile = CreateFileW(logPath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        DWORD dwError = GetLastError();
        return false;
    }

    DWORD dwRes = SetFilePointer(m_hFile, 0, NULL, FILE_END); // 将文件写入指针置到文件末尾
    if (INVALID_SET_FILE_POINTER == dwRes) // 如果置到文件末尾失败，返回false，避免覆盖原日志内容
    {
        DWORD dwError = GetLastError();
        MemoryUtils::SafeCloseHandle(m_hFile, INVALID_HANDLE_VALUE);
        return false;
    }

    // 设置最小写入日志级别
    SetLogLevel(minLevel);
    return true;
}

void Logger::SetLogLevel(LogLevel level)
{
    m_logLevel = level;
}

void Logger::Debug(std::wstring format, ...)
{
    // 声明并初始化可变参数列表
    va_list args;
    va_start(args, format);

    // 计算格式化后的字符串长度
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // 返回-1说明执行错误
    {
        return;
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
        return;
    }

    WriteLog(LogLevel::Debug, std::wstring(vecBuffer.data(), nLen));
}

void Logger::Info(std::wstring format, ...)
{
    // 声明并初始化可变参数列表
    va_list args;
    va_start(args, format);

    // 计算格式化后的字符串长度
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // 返回-1说明执行错误
    {
        return;
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
        return;
    }

    WriteLog(LogLevel::Info, std::wstring(vecBuffer.data(), nLen));
}

void Logger::Warning(std::wstring format, ...)
{
    // 声明并初始化可变参数列表
    va_list args;
    va_start(args, format);

    // 计算格式化后的字符串长度
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // 返回-1说明执行错误
    {
        return;
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
        return;
    }

    WriteLog(LogLevel::Warning, std::wstring(vecBuffer.data(), nLen));
}

void Logger::Error(std::wstring format, ...)
{
    // 声明并初始化可变参数列表
    va_list args;
    va_start(args, format);

    // 计算格式化后的字符串长度
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // 返回-1说明执行错误
    {
        return;
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
        return;
    }

    WriteLog(LogLevel::Error, std::wstring(vecBuffer.data(), nLen));
}

void Logger::Fatal(std::wstring format, ...)
{
    // 声明并初始化可变参数列表
    va_list args;
    va_start(args, format);

    // 计算格式化后的字符串长度
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // 返回-1说明执行错误
    {
        return;
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
        return;
    }

    WriteLog(LogLevel::Fatal, std::wstring(vecBuffer.data(), nLen));
}

void Logger::WriteLog(LogLevel level, const std::wstring& message)
{
    // 先过滤日志等级并检查日志文件句柄
    if (level < m_logLevel || INVALID_HANDLE_VALUE == m_hFile)
    {
        return;
    }

    // 获取当前时间
    SYSTEMTIME st;
    GetLocalTime(&st);

    std::wstring wstrLevel = L"UNKNOWN";
    // 日志级别映射
    switch (level)
    {
    case LogLevel::Debug:
        wstrLevel = L"DEBUG";
        break;
    case LogLevel::Info:
        wstrLevel = L"INFO";
        break;
    case LogLevel::Warning:
        wstrLevel = L"WARNING";
        break;
    case LogLevel::Error:
        wstrLevel = L"ERROR";
        break;
    case LogLevel::Fatal:
        wstrLevel = L"FATAL";
        break;
    default:
        break;
    }

    // 格式化日志
    std::wstring wstrLog = std::to_wstring(st.wYear) + L"-" +
        std::to_wstring(st.wMonth) + L"-" + std::to_wstring(st.wDay) + L" " +
        std::to_wstring(st.wHour) + L":" + std::to_wstring(st.wMinute) + L":" +
        std::to_wstring(st.wSecond) + L"." + std::to_wstring(st.wMilliseconds) + L" [" +
        wstrLevel + L"] " + message;

    // 添加换行符到日志末尾
    std::string strLog = StringUtils::WideToMultiByte(wstrLog) + "\r\n";
    DWORD dwByteWritten;
    WriteFile(m_hFile, strLog.c_str(), (DWORD)strLog.size(), &dwByteWritten, NULL);
}
