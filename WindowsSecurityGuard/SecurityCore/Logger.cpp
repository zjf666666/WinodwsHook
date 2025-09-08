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
    // C++11��֮��汾��̬�����ĳ�ʼ�����̰߳�ȫ�ģ�ֱ��ʹ��static����һ��logger���󲢷���
    static Logger logger;
    return logger;
}

bool Logger::Initialize(const std::wstring& logPath, LogLevel minLevel)
{
    // ����ļ�����Ѵ��ڣ����þ��
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
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

    DWORD dwRes = SetFilePointer(hFile, 0, NULL, FILE_END); // ���ļ�д��ָ���õ��ļ�ĩβ
    if (INVALID_SET_FILE_POINTER == dwRes) // ����õ��ļ�ĩβʧ�ܣ�����false�����⸲��ԭ��־����
    {
        DWORD dwError = GetLastError();
        return false;
    }

    // ������Сд����־����
    SetLogLevel(minLevel);
    return true;
}
