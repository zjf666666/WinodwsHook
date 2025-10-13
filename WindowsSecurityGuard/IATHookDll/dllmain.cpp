#include <Windows.h>
#include <string>
#include <fstream>
#include <sstream>

#include "../HookEngine/IATHook.h"
#include "../include/common/Param.h"

// ԭʼ CreateFile ����ָ�����Ͷ���
typedef HANDLE(WINAPI* pfnCreateFileW)(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPVOID lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

// ����ԭʼ CreateFile ������ַ
pfnCreateFileW g_pfnOrigCreateFileW = NULL;

IATHook g_IATHook;

#pragma optimize("", off)
// Hook �� CreateFile ����ʵ��
HANDLE WINAPI MyCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPVOID lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) {
    // �����д��������ļ���Ҫ����
    if (dwDesiredAccess & (GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA))
    {
        ULONG uTest = dwDesiredAccess & (GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA);
        SetLastError(ERROR_ACCESS_DENIED);
        return INVALID_HANDLE_VALUE;
    }

    g_pfnOrigCreateFileW = (pfnCreateFileW)g_IATHook.GetOriginalFuncAddress();

    // ����ԭʼ����
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

// ��װ Hook
bool InstallHook()
{
    Param param;
    param.Set("common_architecture", std::string("x64"));
    param.Set("iat_function_address", (void*)MyCreateFileW);
    param.Set("common_target_module", std::wstring(L"kernel32.dll"));
    param.Set("iat_function_name", std::string("CreateFileW"));
    g_IATHook.Init(param);
    bool bRes = g_IATHook.Install();
    if (!bRes)
    {
        return false;
    }

    return true;
}

// ж�� Hook
bool UninstallHook() {
    // ����Ӧ��ʹ����Ŀ�е� HookEngine ��ж�� Hook
    // �����޷�ֱ�ӵ�����Ŀ�е� HookEngine���������ʾ��

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
        // ж�� Hook
        UninstallHook();
        break;
    }

    return TRUE;
}