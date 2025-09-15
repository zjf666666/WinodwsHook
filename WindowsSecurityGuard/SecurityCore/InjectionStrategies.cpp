#include "pch.h"
#include "InjectionStrategies.h"
#include "Logger.h"
#include "HandleWrapper.h"
#include "VirtualMemoryWrapper.h"

bool CreateRemoteThreadStrategy::Inject(DWORD pid, const std::wstring& dllPath)
{
    HandleWrapper<> hProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid));
    SIZE_T sizeWrite = 0, sizeDllPath = (dllPath.size() + 1) * sizeof(wchar_t);
    // 获取进程句柄
    if (!hProcess.IsValid())
    {
        Logger::GetInstance().Error(L"OpenProcess failed! error = %d", GetLastError());
        return false;
    }

    // 在进程内部分配写入的内存空间
    VirtualAllocWrapper virAlloc(hProcess.Get(), nullptr, sizeDllPath, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!virAlloc.IsValid())
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    // 将dll地址写入分配空间
    if (FALSE == WriteProcessMemory(hProcess.Get(), virAlloc.Get(), dllPath.c_str(), sizeDllPath, &sizeWrite))
    {
        Logger::GetInstance().Error(L"WriteProcessMemory failed! error = %d", GetLastError());
        return false;
    }

    // 获取loadlibrary函数地址
    FARPROC proc = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    if (nullptr == proc)
    {
        Logger::GetInstance().Error(L"GetProcAddress failed! error = %d", GetLastError());
        return false;
    }

    // 远程线程注入
    HandleWrapper<> hRemote(CreateRemoteThread(hProcess.Get(), NULL, NULL, (LPTHREAD_START_ROUTINE)proc, virAlloc.Get(), NULL, NULL));
    if (!hRemote.IsValid())
    {
        Logger::GetInstance().Error(L"CreateRemoteThread failed! error = %d", GetLastError());
        return false;
    }

    // 等待线程执行结束，这里的错误后续可能需要增加细节处理
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
