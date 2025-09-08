#include <windows.h>
#include "inlineHook.h"
#include <fstream>
#include "../../../interface/logRecord.h"

static inlineHook gs_objMyHook;

BOOL
WINAPI
MyCreateProcess(
    _In_opt_ LPCWSTR lpApplicationName,
    _Inout_opt_ LPWSTR lpCommandLine,
    _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
    _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_ BOOL bInheritHandles,
    _In_ DWORD dwCreationFlags,
    _In_opt_ LPVOID lpEnvironment,
    _In_opt_ LPCWSTR lpCurrentDirectory,
    _In_ LPSTARTUPINFOW lpStartupInfo,
    _Out_ LPPROCESS_INFORMATION lpProcessInformation
)
{
    //gs_objMyHook.unistallMyHook();
    //logRecord::getInstance()->writeMsg(L"HOOK CREATEPROCESS", LOGTYPE::LEVEL_DEBUG);
    //MessageBox(NULL, L"CreateProcessW被HOOK了！", L"HaHaHa", MB_SYSTEMMODAL);
    ////CreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
    ////    dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    return false;
}

int WINAPI MyMessageBoxA(
    HWND   hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT   uType
)
{
    gs_objMyHook.unistallMyHookByName(); // 调用原函数前需要解除hook
    int result = MessageBoxA(hWnd, "INLINE HOOK", "INLINE HOOK", MB_OK);
    gs_objMyHook.reInstallMyHookByName(); // 如果还需要继续hook这个函数 则重新hook
    return result;
}


BOOL WINAPI DllMain(
    HMODULE hModule,             // 指向自身的句柄
    DWORD   ul_reason_for_call,  // 调用原因
    LPVOID  lpReserved           // 隐式加载和显式加载
)
{
    logRecord::getInstance()->initLog(L"C:\\ProgramData\\inlinhook.txt");
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: // dll加载
        gs_objMyHook.myHookByName((char*)MessageBoxA, (DWORD)MyMessageBoxA);
        //gs_objMyHook.myHookByAddr(L"user32.dll", "MessageBoxA", (DWORD)MyMessageBoxA);
        break;
    case DLL_PROCESS_DETACH: // dll卸载
        gs_objMyHook.unistallMyHookByAddr();
        break;
    }
    return true; // 没有这个return true会导致无法注入成功
}

//#include "pch.h"

//我们这次hook的是MessageBoxA

//DWORD g_oldFuncAddr = 0;
//BYTE g_oldCode[5] = { 0 };
//BYTE g_newCode[5] = { 0xE9 };
//
//BOOL initHook();
//bool installHook();
//bool unistallHook();
//
//int WINAPI MyMessageBoxA(
//    HWND   hWnd,
//    LPCSTR lpText,
//    LPCSTR lpCaption,
//    UINT   uType
//)
//{
//    unistallHook();
//    int result = MessageBoxA(hWnd, "本活动由小鬼哥哥赞助", "太裤剌！", MB_OK);
//    installHook();
//    return result;
//}
//
//BOOL initHook() {
//
//    //获取需要hook的函数MessageBoxA的地址
//    HMODULE hDll = LoadLibraryA("user32.dll");
//    g_oldFuncAddr = (DWORD)GetProcAddress(hDll, "MessageBoxA");
//    if (g_oldFuncAddr > 0) {
//        MessageBoxA(0, "找到了MessageBoxA函数", "标题", MB_OK);
//    }
//
//    //保留原汇编代码
//    memcpy(g_oldCode, (char*)MessageBoxA, 5);
//
//    //保存新汇编代码
//    DWORD offset = (DWORD)MyMessageBoxA - (DWORD)MessageBoxA - 5;
//    memcpy(&g_newCode[1], &offset, 4);
//
//    return true;
//}
//
//bool installHook() {
//
//    DWORD oldProtect = 0;
//    VirtualProtect((char*)MessageBoxA, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
//    memcpy((char*)MessageBoxA, g_newCode, 5);
//    VirtualProtect((char*)g_oldFuncAddr, 5, oldProtect, &oldProtect);
//    return true;
//}
//
//bool unistallHook() {
//    DWORD oldProtect = 0;
//    VirtualProtect((char*)MessageBoxA, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
//    memcpy((char*)MessageBoxA, g_oldCode, 5);
//    VirtualProtect((char*)MessageBoxA, 5, oldProtect, &oldProtect);
//
//    return true;
//}
//
//BOOL APIENTRY DllMain(HMODULE hModule,
//    DWORD  ul_reason_for_call,
//    LPVOID lpReserved
//)
//{
//    switch (ul_reason_for_call)
//    {
//    case DLL_PROCESS_ATTACH: {
//        initHook();
//        bool bret = installHook();
//        if (bret) {
//            MessageBoxW(0, L"钩子安装成功！", L"标题", MB_OK);
//        }
//        break;
//    }
//    case DLL_THREAD_ATTACH:
//        break;
//    case DLL_THREAD_DETACH:
//        break;
//    case DLL_PROCESS_DETACH: {
//        bool bret = unistallHook();
//        if (bret) {
//            MessageBoxW(0, L"钩子卸载成功", L"标题", MB_OK);
//        }
//        break;
//    }
//    }
//    return TRUE;
//}