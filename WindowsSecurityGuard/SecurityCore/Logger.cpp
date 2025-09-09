#include "pch.h"
#include "Logger.h"

// windows��
#include <Shlobj.h>

// ����������
#include "StringUtils.h"
#include "Constants.h"

Logger::Logger() : m_hFile(INVALID_HANDLE_VALUE) {}

Logger::~Logger()
{
    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

Logger& Logger::GetInstance()
{
    // C++11��֮��汾��̬�����ĳ�ʼ�����̰߳�ȫ�ģ�ֱ��ʹ��static����һ��logger���󲢷���
    static Logger logger;
    return logger;
}

bool Logger::Initialize(const std::wstring& logPath, LogLevel minLevel)
{
    // ����ļ�����Ѵ��ڣ����þ��
    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    // ����Ŀ¼
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

    // �ļ��������Ҵ���ʧ���򷵻ش���
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

    DWORD dwRes = SetFilePointer(m_hFile, 0, NULL, FILE_END); // ���ļ�д��ָ���õ��ļ�ĩβ
    if (INVALID_SET_FILE_POINTER == dwRes) // ����õ��ļ�ĩβʧ�ܣ�����false�����⸲��ԭ��־����
    {
        DWORD dwError = GetLastError();
        return false;
    }

    // ������Сд����־����
    SetLogLevel(minLevel);
    return true;
}

void Logger::SetLogLevel(LogLevel level)
{
    m_logLevel = level;
}

void Logger::Debug(std::wstring format, ...)
{
    // ��������ʼ���ɱ�����б�
    va_list args;
    va_start(args, format);

    // �����ʽ������ַ�������
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // ����-1˵��ִ�д���
    {
        return;
    }

    // argsֻ��ʹ��һ�Σ�ÿ��ʹ�ú���Ҫ�ͷŲ�����start
    va_start(args, format);

    // �����㹻�Ļ�����
    std::vector<wchar_t> vecBuffer(nLen + 1, 0);

    // ʹ��vswprintf_sִ�и�ʽ��
    int nRes = vswprintf_s(vecBuffer.data(), nLen + 1, format.c_str(), args);

    // ����ɱ�����б�
    va_end(args);
    if (LENGTH_MINUS_ONE == nRes)
    {
        return;
    }

    WriteLog(LogLevel::Debug, std::wstring(vecBuffer.data(), nLen));
}

void Logger::Info(std::wstring format, ...)
{
    // ��������ʼ���ɱ�����б�
    va_list args;
    va_start(args, format);

    // �����ʽ������ַ�������
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // ����-1˵��ִ�д���
    {
        return;
    }

    // argsֻ��ʹ��һ�Σ�ÿ��ʹ�ú���Ҫ�ͷŲ�����start
    va_start(args, format);

    // �����㹻�Ļ�����
    std::vector<wchar_t> vecBuffer(nLen + 1, 0);

    // ʹ��vswprintf_sִ�и�ʽ��
    int nRes = vswprintf_s(vecBuffer.data(), nLen + 1, format.c_str(), args);

    // ����ɱ�����б�
    va_end(args);
    if (LENGTH_MINUS_ONE == nRes)
    {
        return;
    }

    WriteLog(LogLevel::Info, std::wstring(vecBuffer.data(), nLen));
}

void Logger::Warning(std::wstring format, ...)
{
    // ��������ʼ���ɱ�����б�
    va_list args;
    va_start(args, format);

    // �����ʽ������ַ�������
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // ����-1˵��ִ�д���
    {
        return;
    }

    // argsֻ��ʹ��һ�Σ�ÿ��ʹ�ú���Ҫ�ͷŲ�����start
    va_start(args, format);

    // �����㹻�Ļ�����
    std::vector<wchar_t> vecBuffer(nLen + 1, 0);

    // ʹ��vswprintf_sִ�и�ʽ��
    int nRes = vswprintf_s(vecBuffer.data(), nLen + 1, format.c_str(), args);

    // ����ɱ�����б�
    va_end(args);
    if (LENGTH_MINUS_ONE == nRes)
    {
        return;
    }

    WriteLog(LogLevel::Warning, std::wstring(vecBuffer.data(), nLen));
}

void Logger::Error(std::wstring format, ...)
{
    // ��������ʼ���ɱ�����б�
    va_list args;
    va_start(args, format);

    // �����ʽ������ַ�������
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // ����-1˵��ִ�д���
    {
        return;
    }

    // argsֻ��ʹ��һ�Σ�ÿ��ʹ�ú���Ҫ�ͷŲ�����start
    va_start(args, format);

    // �����㹻�Ļ�����
    std::vector<wchar_t> vecBuffer(nLen + 1, 0);

    // ʹ��vswprintf_sִ�и�ʽ��
    int nRes = vswprintf_s(vecBuffer.data(), nLen + 1, format.c_str(), args);

    // ����ɱ�����б�
    va_end(args);
    if (LENGTH_MINUS_ONE == nRes)
    {
        return;
    }

    WriteLog(LogLevel::Error, std::wstring(vecBuffer.data(), nLen));
}

void Logger::Fatal(std::wstring format, ...)
{
    // ��������ʼ���ɱ�����б�
    va_list args;
    va_start(args, format);

    // �����ʽ������ַ�������
    int nLen = _vscwprintf(format.c_str(), args);
    va_end(args);
    if (LENGTH_MINUS_ONE == nLen)  // ����-1˵��ִ�д���
    {
        return;
    }

    // argsֻ��ʹ��һ�Σ�ÿ��ʹ�ú���Ҫ�ͷŲ�����start
    va_start(args, format);

    // �����㹻�Ļ�����
    std::vector<wchar_t> vecBuffer(nLen + 1, 0);

    // ʹ��vswprintf_sִ�и�ʽ��
    int nRes = vswprintf_s(vecBuffer.data(), nLen + 1, format.c_str(), args);

    // ����ɱ�����б�
    va_end(args);
    if (LENGTH_MINUS_ONE == nRes)
    {
        return;
    }

    WriteLog(LogLevel::Fatal, std::wstring(vecBuffer.data(), nLen));
}

void Logger::WriteLog(LogLevel level, const std::wstring& message)
{
    // �ȹ�����־�ȼ��������־�ļ����
    if (level < m_logLevel || INVALID_HANDLE_VALUE == m_hFile)
    {
        return;
    }

    // ��ȡ��ǰʱ��
    SYSTEMTIME st;
    GetLocalTime(&st);

    std::wstring wstrLevel = L"UNKNOWN";
    // ��־����ӳ��
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

    // ��ʽ����־
    std::wstring wstrLog = std::to_wstring(st.wYear) + L"-" +
        std::to_wstring(st.wMonth) + L"-" + std::to_wstring(st.wDay) + L" " +
        std::to_wstring(st.wHour) + L":" + std::to_wstring(st.wMinute) + L":" +
        std::to_wstring(st.wSecond) + L"." + std::to_wstring(st.wMilliseconds) + L" [" +
        wstrLevel + L"] " + message;

    std::string strLog = StringUtils::WideToMultiByte(wstrLog);
    DWORD dwByteWritten;
    WriteFile(m_hFile, strLog.c_str(), (DWORD)strLog.size(), &dwByteWritten, NULL);
}