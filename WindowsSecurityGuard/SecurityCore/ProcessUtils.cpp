#include "pch.h"
#include "ProcessUtils.h"

// windows��
#include <psapi.h>
#include <tlhelp32.h>

// ����������
#include "Logger.h"
#include "MemoryUtils.h"

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
        MemoryUtils::SafeCloseHandle(hModule);
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

    MemoryUtils::SafeCloseHandle(hModule);
    return vecRes;
}

bool ProcessUtils::ElevatePrivileges(const std::wstring& privilege)
{
    /* ʹ�����Ʒ�ʽ��Ȩ����Ҫ�����Թ���Ա������� */

    // ��ȡ��ǰ���̾����GetCurrentProcess���ص���α�����-1������������Ҫ�ͷ�
    HANDLE hProcess = GetCurrentProcess();

    // ��ȡ�������� TOKEN_ADJUST_PRIVILEGES ����adjustTokenPrivileges������Ҫ���Ȩ��
    HANDLE hToken = NULL;
    if (FALSE == OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        Logger::GetInstance().Error(L"OpenProcessToken failed! error = %d", GetLastError());
        return false;
    }

    // ��ȡ����ϵͳ����ȨLUIDֵ
    LUID luid;
    if (FALSE == LookupPrivilegeValue(NULL, privilege.c_str(), &luid))
    {
        Logger::GetInstance().Error(L"LookupPrivilegeValue failed! error = %d", GetLastError());
        MemoryUtils::SafeCloseHandle(hToken);
        return false;
    }

    // ���ý���Ȩ��
    TOKEN_PRIVILEGES tokenPrivileges = { 0 };
    tokenPrivileges.PrivilegeCount = ANYSIZE_ARRAY; // ���ֵ��Ӧ��LUID�����������ǹ̶���1
    tokenPrivileges.Privileges[0].Luid = luid;
    if (FALSE == AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, NULL, NULL))
    {
        Logger::GetInstance().Error(L"AdjustTokenPrivileges failed! error = %d", GetLastError());
        MemoryUtils::SafeCloseHandle(hToken);
        return false;
    }

    MemoryUtils::SafeCloseHandle(hToken);
    return true;
}
