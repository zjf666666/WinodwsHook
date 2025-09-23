#include <Windows.h>
#include <string>
#include <fstream>
#include <sstream>

#include "../HookEngine/InlineHook.h"

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

InlineHook* g_pInlineHook = nullptr;

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
    //if (dwDesiredAccess & (GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA))
    //{
    //    ULONG uTest = dwDesiredAccess & (GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA);
    //    SetLastError(ERROR_ACCESS_DENIED);
    //    return INVALID_HANDLE_VALUE;
    //}

    SetLastError(0x7777);

    DWORD dwError1 = GetLastError();

    g_pfnOrigCreateFileW = (pfnCreateFileW)g_pInlineHook->GetTrampolineAddress();
    // DWORD dwError2 = GetLastError();
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
    DWORD dwError3 = GetLastError();
    //SetLastError(0x7777);

    return hHandle;
}
#pragma optimize("", on)

// 安装 Hook
bool InstallHook() {

    HMODULE hModule = GetModuleHandleW(L"kernelBase.dll");
    if (nullptr == hModule)
    {
        DWORD dwError = GetLastError();
    }
    g_pfnOrigCreateFileW = (pfnCreateFileW)GetProcAddress(hModule, "CreateFileW");

    g_pInlineHook = new InlineHook(L"kernel32.dll", "CreateFileW", (void*)MyCreateFileW, true);
    bool bRes = g_pInlineHook->Install();
    if (!bRes) {
        delete g_pInlineHook;
        g_pInlineHook = nullptr;
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