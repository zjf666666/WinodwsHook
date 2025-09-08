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
    Logger() {} // ���캯��˽�У���ֹ�ⲿ��������
    ~Logger() {} // ��������˽�У���ֹ�ⲿ��������

    Logger(const Logger& logger) = delete; // ɾ���������캯��
    Logger& operator=(const Logger& logger) = delete; // ɾ����������

private:
    HANDLE hFile;
};

