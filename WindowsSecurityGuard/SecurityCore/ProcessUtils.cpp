#include "pch.h"
#include "ProcessUtils.h"

// windows��
#include <psapi.h>
#include <tlhelp32.h>

// ����������
#include "Logger.h"

std::vector<DWORD> ProcessUtils::EnumProcess()
{
    DWORD dwProcessesPID[1024], dwCbNeeded;

    if (!EnumProcesses(dwProcessesPID, sizeof(dwProcessesPID), &dwCbNeeded))
    {
        Logger::GetInstance().Error(L"enumprocesses failed! error = %d", GetLastError());
        return {};
    }

    int nLen = dwCbNeeded / sizeof(DWORD);

    std::vector<DWORD> vecRes(dwProcessesPID, dwProcessesPID + nLen);
    return vecRes;
}

ProcessInfo ProcessUtils::GetProcessInfo(DWORD pid)
{
    ProcessInfo objRes;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (NULL == hProcess)
    {
        Logger::GetInstance().Error(L"OpenProcess failed! pid = %d, error = %d", pid, GetLastError);
        return objRes;
    }
    
    wchar_t wszPath[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    // visit��server2008֮��汾֧�֣��ϰ汾ʹ��GetProcessImageFileName+QueryDosDeviceʵ��
    if (0 == QueryFullProcessImageName(hProcess, 0, wszPath, &dwSize))
    {
        Logger::GetInstance().Error(L"QueryFullProcessImageName failed! pid = %d, error = %d", pid, GetLastError);
        CloseHandle(hProcess);
        return objRes;
    }

    CloseHandle(hProcess);
    objRes.processId = pid;
    objRes.processName = wszPath;
    return objRes;
}

std::vector<ModuleInfo> ProcessUtils::GetProcessModules(DWORD pid)
{
    HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (INVALID_HANDLE_VALUE == hModule)
    {
        Logger::GetInstance().Error(L"CreateToolhelp32Snapshot failed! pid = %d, error = %d", pid, GetLastError());
        return {};
    }

    MODULEENTRY32 moduleInfo;
    moduleInfo.dwSize = sizeof(MODULEENTRY32);
    if (!Module32FirstW(hModule, &moduleInfo))
    {
        Logger::GetInstance().Error(L"Module32First failed! pid = %d, error = %d", pid, GetLastError());
        CloseHandle(hModule);
        return {};
    }

    std::vector<ModuleInfo> vecRes;

    do
    {
        vecRes.push_back(
            ModuleInfo(
                moduleInfo.szModule,
                moduleInfo.szExePath,
                moduleInfo.modBaseAddr,
                moduleInfo.dwSize,
                moduleInfo.hModule
            )
        );
    } while (Module32NextW(hModule, &moduleInfo));

    CloseHandle(hModule);
    return vecRes;
}
