#include "pch.h"
#include "ProcessUtils.h"

// windows库
#include <psapi.h>
#include <tlhelp32.h>

// 其它依赖库
#include "Logger.h"
#include "MemoryUtils.h"
#include "Constants.h"
#include "HandleWrapper.h"

#define INVALID_PID    0

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

bool ProcessUtils::IsProcessRunning(const std::wstring& processName)
{
    // 通过进程名获取进程ID，如果ID为0表示进程不存在
    DWORD pid = GetProcessIdByName(processName);
    return (pid != 0);
}

ProcessInfo ProcessUtils::GetProcessInfo(DWORD pid)
{
    ProcessInfo objRes;
    HandleWrapper<> hProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid));
    if (!hProcess.IsValid())
    {
        Logger::GetInstance().Error(L"OpenProcess failed! pid = %d, error = %d", pid, GetLastError());
        return objRes;
    }
    
    wchar_t wszPath[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    // visit及server2008之后版本支持，老版本使用GetProcessImageFileName+QueryDosDevice实现
    if (0 == QueryFullProcessImageName(hProcess.Get(), 0, wszPath, &dwSize))
    {
        Logger::GetInstance().Error(L"QueryFullProcessImageName failed! pid = %d, error = %d", pid, GetLastError());
        return objRes;
    }

    objRes.processId = pid;
    objRes.processName = wszPath;
    return objRes;
}

DWORD ProcessUtils::GetProcessIdByName(const std::wstring& processName)
{
    DWORD dwPid = INVALID_PID;

    // 创建进程快照
    HandleWrapper<> hSnapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (!hSnapshot.IsValid())
    {
        Logger::GetInstance().Error(L"CreateToolhelp32Snapshot failed! error = %d", GetLastError());
        return dwPid;
    }

    // 获取第一个进程快照
    PROCESSENTRY32W pe32 = { 0 };
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (!Process32FirstW(hSnapshot.Get(), &pe32))
    {
        Logger::GetInstance().Error(L"Process32FirstW failed! error = %d", GetLastError());
        return dwPid;
    }

    // 遍历进程快照
    do
    {
        if (0 == _wcsicmp(pe32.szExeFile, processName.c_str()))
        {
            dwPid = pe32.th32ProcessID;
            break;
        }
    } while (Process32NextW(hSnapshot.Get(), &pe32));

    return dwPid;
}

std::vector<ModuleInfo> ProcessUtils::GetProcessModules(DWORD pid)
{
    HandleWrapper<> hModule(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid));
    if (!hModule.IsValid())
    {
        Logger::GetInstance().Error(L"CreateToolhelp32Snapshot failed! pid = %d, error = %d", pid, GetLastError());
        return {};
    }

    MODULEENTRY32 moduleInfo;
    moduleInfo.dwSize = sizeof(MODULEENTRY32);
    if (!Module32FirstW(hModule.Get(), &moduleInfo))
    {
        Logger::GetInstance().Error(L"Module32First failed! pid = %d, error = %d", pid, GetLastError());
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
    } while (Module32NextW(hModule.Get(), &moduleInfo));

    return vecRes;
}

bool ProcessUtils::ElevatePrivileges(const std::wstring& privilege)
{
    /* 使用令牌方式提权，需要程序以管理员身份运行 */

    // 获取当前进程句柄，GetCurrentProcess返回的是伪句柄（-1）这个句柄不需要释放
    HANDLE hProcess = GetCurrentProcess();

    // 获取进程令牌 TOKEN_ADJUST_PRIVILEGES 后续adjustTokenPrivileges函数需要这个权限
    HANDLE hTokenOri = nullptr;
    if (FALSE == OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hTokenOri))
    {
        Logger::GetInstance().Error(L"OpenProcessToken failed! error = %d", GetLastError());
        return false;
    }

    HandleWrapper<> hToken(hTokenOri);

    // 获取本地系统的特权LUID值
    LUID luid;
    if (FALSE == LookupPrivilegeValue(nullptr, privilege.c_str(), &luid))
    {
        Logger::GetInstance().Error(L"LookupPrivilegeValue failed! error = %d", GetLastError());
        return false;
    }

    // 设置进程权限
    TOKEN_PRIVILEGES tokenPrivileges = { 0 };
    tokenPrivileges.PrivilegeCount = ANYSIZE_ARRAY; // 这个值对应的LUID数组数量，是固定的1
    tokenPrivileges.Privileges[0].Luid = luid;
    if (FALSE == AdjustTokenPrivileges(hToken.Get(), FALSE, &tokenPrivileges, 0, nullptr, nullptr))
    {
        Logger::GetInstance().Error(L"AdjustTokenPrivileges failed! error = %d", GetLastError());
        return false;
    }

    return true;
}

int ProcessUtils::IsProcess64Bit(DWORD pid)
{
    HandleWrapper<> hProcess(OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid));
    if (!hProcess.IsValid())
    {
        Logger::GetInstance().Error(L"OpenProcess failed! error = %d", GetLastError());
        return ERROR_OPEN_PROCESS;
    }

    // 判断文件是否为64位进程
    BOOL bIsWOW64 = FALSE;
    if (!IsWow64Process(hProcess.Get(), &bIsWOW64))
    {
        Logger::GetInstance().Error(L"IsWow64Process failed! error = %d", GetLastError());
        return ERROR_IS_WOW64_PROCESS;
    }

    // 判断系统架构
    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo(&systemInfo);
    bool bIsSystem64Bit = (PROCESSOR_ARCHITECTURE_ARM64 == systemInfo.wProcessorArchitecture || 
        PROCESSOR_ARCHITECTURE_AMD64 == systemInfo.wProcessorArchitecture);

    // 只有64位系统且标志位为FALSE才是64为进程
    if (bIsSystem64Bit && FALSE == bIsWOW64)
    {
        return ARCHITECTURE_64;
    }
    return ARCHITECTURE_32; // 其他情况均为32位进程
}

std::wstring ProcessUtils::GetCurrentProcessDir()
{
    std::vector<wchar_t> vecPath(MAX_PATH, 0);
    DWORD dwLen = GetModuleFileNameW(nullptr, vecPath.data(), vecPath.size());

    if (dwLen == vecPath.size() && ERROR_INSUFFICIENT_BUFFER == GetLastError())
    {
        vecPath.resize(vecPath.size() * 2);
        dwLen = GetModuleFileNameW(nullptr, vecPath.data(), vecPath.size());
    }

    if (0 == dwLen)
    {
        return L"";
    }

    std::wstring wstrPath(vecPath.begin(), vecPath.end());
    size_t lastBackslash = wstrPath.find_last_of(L"\\/");
    if (lastBackslash == std::wstring::npos)
    {
        return L"";
    }
    std::wstring exeDir = wstrPath.substr(0, lastBackslash + 1);
    return exeDir;
}
