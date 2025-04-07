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
    //MessageBox(NULL, L"CreateProcessW��HOOK�ˣ�", L"HaHaHa", MB_SYSTEMMODAL);
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
    gs_objMyHook.unistallMyHookByName(); // ����ԭ����ǰ��Ҫ���hook
    int result = MessageBoxA(hWnd, "INLINE HOOK", "INLINE HOOK", MB_OK);
    gs_objMyHook.reInstallMyHookByName(); // �������Ҫ����hook������� ������hook
    return result;
}


BOOL WINAPI DllMain(
    HMODULE hModule,             // ָ������ľ��
    DWORD   ul_reason_for_call,  // ����ԭ��
    LPVOID  lpReserved           // ��ʽ���غ���ʽ����
)
{
    logRecord::getInstance()->initLog(L"C:\\ProgramData\\inlinhook.txt");
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: // dll����
        gs_objMyHook.myHookByName((char*)MessageBoxA, (DWORD)MyMessageBoxA);
        //gs_objMyHook.myHookByAddr(L"user32.dll", "MessageBoxA", (DWORD)MyMessageBoxA);
        break;
    case DLL_PROCESS_DETACH: // dllж��
        gs_objMyHook.unistallMyHookByAddr();
        break;
    }
    return true; // û�����return true�ᵼ���޷�ע��ɹ�
}

//#include "pch.h"

//�������hook����MessageBoxA

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
//    int result = MessageBoxA(hWnd, "�����С��������", "̫���ݣ�", MB_OK);
//    installHook();
//    return result;
//}
//
//BOOL initHook() {
//
//    //��ȡ��Ҫhook�ĺ���MessageBoxA�ĵ�ַ
//    HMODULE hDll = LoadLibraryA("user32.dll");
//    g_oldFuncAddr = (DWORD)GetProcAddress(hDll, "MessageBoxA");
//    if (g_oldFuncAddr > 0) {
//        MessageBoxA(0, "�ҵ���MessageBoxA����", "����", MB_OK);
//    }
//
//    //����ԭ������
//    memcpy(g_oldCode, (char*)MessageBoxA, 5);
//
//    //�����»�����
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
//            MessageBoxW(0, L"���Ӱ�װ�ɹ���", L"����", MB_OK);
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
//            MessageBoxW(0, L"����ж�سɹ�", L"����", MB_OK);
//        }
//        break;
//    }
//    }
//    return TRUE;
//}