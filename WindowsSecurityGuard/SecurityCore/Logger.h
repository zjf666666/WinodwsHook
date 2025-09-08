#pragma once
#include <string>
#include <Windows.h>

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger
{
public:
    static Logger& GetInstance();

    bool Initialize(const std::wstring& logPath, LogLevel minLevel = LogLevel::Info);

    void SetLogLevel(LogLevel level);

    void Debug(const std::wstring& message);
    void Info(const std::wstring& message);
    void Warning(const std::wstring& message);
    void Error(const std::wstring& message);
    void Fatal(const std::wstring& message);

private:
    Logger() {} // 构造函数私有，防止外部创建对象
    ~Logger() {} // 析构函数私有，防止外部析构对象

    Logger(const Logger& logger) = delete; // 删除拷贝构造函数
    Logger& operator=(const Logger& logger) = delete; // 删除拷贝操作

private:
    HANDLE hFile;
};

