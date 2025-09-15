#include "pch.h"
#include "InjectionStrategies.h"
#include "Logger.h"
#include "HandleWrapper.h"
#include "VirtualMemoryWrapper.h"

bool CreateRemoteThreadStrategy::Inject(DWORD pid, const std::wstring& dllPath)
{
    HandleWrapper<> hProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid));
    SIZE_T sizeWrite = 0, sizeDllPath = (dllPath.size() + 1) * sizeof(wchar_t);
    // ��ȡ���̾��
    if (!hProcess.IsValid())
    {
        Logger::GetInstance().Error(L"OpenProcess failed! error = %d", GetLastError());
        return false;
    }

    // �ڽ����ڲ�����д����ڴ�ռ�
    VirtualAllocWrapper virAlloc(hProcess.Get(), nullptr, sizeDllPath, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!virAlloc.IsValid())
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    // ��dll��ַд�����ռ�
    if (FALSE == WriteProcessMemory(hProcess.Get(), virAlloc.Get(), dllPath.c_str(), sizeDllPath, &sizeWrite))
    {
        Logger::GetInstance().Error(L"WriteProcessMemory failed! error = %d", GetLastError());
        return false;
    }

    // ��ȡloadlibrary������ַ
    FARPROC proc = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    if (nullptr == proc)
    {
        Logger::GetInstance().Error(L"GetProcAddress failed! error = %d", GetLastError());
        return false;
    }

    // Զ���߳�ע��
    HandleWrapper<> hRemote(CreateRemoteThread(hProcess.Get(), NULL, NULL, (LPTHREAD_START_ROUTINE)proc, virAlloc.Get(), NULL, NULL));
    if (!hRemote.IsValid())
    {
        Logger::GetInstance().Error(L"CreateRemoteThread failed! error = %d", GetLastError());
        return false;
    }

    // �ȴ��߳�ִ�н���������Ĵ������������Ҫ����ϸ�ڴ���
    DWORD dwRes = WaitForSingleObject(hRemote.Get(), 5000);
    if (WAIT_OBJECT_0 != dwRes)
    {
        if (WAIT_FAILED == dwRes)
        {
            Logger::GetInstance().Error(L"WaitForSingleObject failed! error = %d", GetLastError());
        }
        else
        {
            Logger::GetInstance().Warning(L"WaitForSingleObject failed! error = %d", dwRes);
        }
        return false;
    }

    return true;
}

bool CreateRemoteThreadStrategy::Eject(DWORD pid, const std::wstring& dllPath)
{
    return false;
}

std::wstring CreateRemoteThreadStrategy::GetStrategyName() const
{
    return L"CreateRemoteThread";
}

bool QueueUserAPCStrategy::Inject(DWORD pid, const std::wstring& dllPath)
{
    return false;
}

bool QueueUserAPCStrategy::Eject(DWORD pid, const std::wstring& dllPath)
{
    return false;
}

std::wstring QueueUserAPCStrategy::GetStrategyName() const
{
    return L"QueueUserAPC";
}
