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

    void Debug(std::wstring format, ...);
    void Info(std::wstring format, ...);
    void Warning(std::wstring format, ...);
    void Error(std::wstring format, ...);
    void Fatal(std::wstring format, ...);

private:
    Logger() {} // ���캯��˽�У���ֹ�ⲿ��������
    ~Logger() {} // ��������˽�У���ֹ�ⲿ��������

    Logger(const Logger& logger) = delete; // ɾ���������캯��
    Logger& operator=(const Logger& logger) = delete; // ɾ����������

private:
    void WriteLog(LogLevel level, const std::wstring& message);

private:
    HANDLE m_hFile;
    LogLevel m_logLevel;
};

