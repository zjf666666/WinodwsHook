#include <Windows.h>
#include <string>
#include <fstream>
#include <sstream>

#include "../HookEngine/IATHook.h"
#include "../HookEngine/HookParam.h"

// 原始 CreateFile 函数指针类型定义
typedef HANDLE(WINAPI* pfnCreateFileW)(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPVOID lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

// 保存原始 CreateFile 函数地址
pfnCreateFileW g_pfnOrigCreateFileW = NULL;

IATHook g_IATHook;

#pragma optimize("", off)
// Hook 的 CreateFile 函数实现
HANDLE WINAPI MyCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPVOID lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) {
    // 如果是写入操作且文件需要保护
    if (dwDesiredAccess & (GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA))
    {
        ULONG uTest = dwDesiredAccess & (GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA);
        SetLastError(ERROR_ACCESS_DENIED);
        return INVALID_HANDLE_VALUE;
    }

    g_pfnOrigCreateFileW = (pfnCreateFileW)g_IATHook.GetOriginalFuncAddress();

    // 调用原始函数
    HANDLE hHandle = g_pfnOrigCreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile
    );

    return 0;
}
#pragma optimize("", on)

// 安装 Hook
bool InstallHook()
{
    IATHookParam param;
    param.targetModule = L"kernel32.dll";
    param.bIs64Bit = true;
    param.pHookFunction = MyCreateFileW;
    param.targetFunction = "CreateFileW";
    g_IATHook.Init(&param);
    bool bRes = g_IATHook.Install();
    if (!bRes)
    {
        return false;
    }

    return true;
}

// 卸载 Hook
bool UninstallHook() {
    // 这里应该使用项目中的 HookEngine 来卸载 Hook
    // 由于无法直接调用项目中的 HookEngine，这里仅作示例

    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        int a = 1;
        InstallHook();
    }
    break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        // 卸载 Hook
        UninstallHook();
        break;
    }

    return TRUE;
}