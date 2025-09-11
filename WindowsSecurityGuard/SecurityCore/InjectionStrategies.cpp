#include "pch.h"
#include "InjectionStrategies.h"
#include "Logger.h"
#include "MemoryUtils.h"

bool CreateRemoteThreadStrategy::Inject(DWORD pid, const std::wstring& dllPath)
{
    bool bRes = false;
    HANDLE hProcess = nullptr, hRemote = nullptr;
    LPVOID lpAddr = nullptr;
    SIZE_T sizeWrite = 0, sizeDllPath = (dllPath.size() + 1) * sizeof(wchar_t);
    // ��ȡ���̾��
    do
    {
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (nullptr == hProcess)
        {
            Logger::GetInstance().Error(L"OpenProcess failed! error = %d", GetLastError());
            break;
        }

        // �ڽ����ڲ�����д����ڴ�ռ�
        lpAddr = VirtualAllocEx(hProcess, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (nullptr == lpAddr)
        {
            Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
            break;
        }

        // ��dll��ַд�����ռ�
        if (FALSE == WriteProcessMemory(hProcess, lpAddr, dllPath.c_str(), sizeDllPath, &sizeWrite))
        {
            Logger::GetInstance().Error(L"WriteProcessMemory failed! error = %d", GetLastError());
            break;
        }

        // ��ȡloadlibrary������ַ
        FARPROC proc = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
        if (nullptr == proc)
        {
            Logger::GetInstance().Error(L"GetProcAddress failed! error = %d", GetLastError());
            break;
        }

        // Զ���߳�ע��
        hRemote = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)proc, lpAddr, NULL, NULL);
        if (nullptr == hRemote)
        {
            Logger::GetInstance().Error(L"CreateRemoteThread failed! error = %d", GetLastError());
            break;
        }

        // �ȴ��߳�ִ�н���������Ĵ������������Ҫ����ϸ�ڴ���
        DWORD dwRes = WaitForSingleObject(hRemote, 5000);
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
            break;
        }
        bRes = true;
    } while (false);
    
    // ���ͷ���Դ
    if (nullptr != lpAddr)
    {
        VirtualFreeEx(hProcess, lpAddr, sizeDllPath, MEM_RELEASE);
    }

    // �ͷ�remote���
    MemoryUtils::SafeCloseHandle(hRemote);

    // ����ͷŽ��̾��
    MemoryUtils::SafeCloseHandle(hProcess);
    return bRes;
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
