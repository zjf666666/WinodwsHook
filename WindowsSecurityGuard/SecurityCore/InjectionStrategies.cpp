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
    // 获取进程句柄
    do
    {
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (nullptr == hProcess)
        {
            Logger::GetInstance().Error(L"OpenProcess failed! error = %d", GetLastError());
            break;
        }

        // 在进程内部分配写入的内存空间
        lpAddr = VirtualAllocEx(hProcess, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (nullptr == lpAddr)
        {
            Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
            break;
        }

        // 将dll地址写入分配空间
        if (FALSE == WriteProcessMemory(hProcess, lpAddr, dllPath.c_str(), sizeDllPath, &sizeWrite))
        {
            Logger::GetInstance().Error(L"WriteProcessMemory failed! error = %d", GetLastError());
            break;
        }

        // 获取loadlibrary函数地址
        FARPROC proc = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
        if (nullptr == proc)
        {
            Logger::GetInstance().Error(L"GetProcAddress failed! error = %d", GetLastError());
            break;
        }

        // 远程线程注入
        hRemote = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)proc, lpAddr, NULL, NULL);
        if (nullptr == hRemote)
        {
            Logger::GetInstance().Error(L"CreateRemoteThread failed! error = %d", GetLastError());
            break;
        }

        // 等待线程执行结束，这里的错误后续可能需要增加细节处理
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
    
    // 先释放资源
    if (nullptr != lpAddr)
    {
        VirtualFreeEx(hProcess, lpAddr, sizeDllPath, MEM_RELEASE);
    }

    // 释放remote句柄
    MemoryUtils::SafeCloseHandle(hRemote);

    // 最后释放进程句柄
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
