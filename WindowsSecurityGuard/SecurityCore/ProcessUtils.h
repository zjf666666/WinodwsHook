#pragma once
#include <vector>
#include <string>
#include <Windows.h>

// ��ʱֻ��ȡ��processId �� processName
struct ProcessInfo {
    DWORD processId;           // ����ID
    std::wstring processName;  // ��������
    std::wstring fullPath;     // ��������·��
    DWORD parentProcessId;     // ������ID
    DWORD threadCount;         // �߳�����
    DWORD priorityClass;       // ���ȼ�
    bool is64Bit;              // �Ƿ�Ϊ64λ����
    FILETIME creationTime;     // ����ʱ��
};

struct ModuleInfo {
    std::wstring moduleName;   // ģ������
    std::wstring fullPath;     // ģ������·��
    BYTE* baseAddress;         // ģ���ַ
    DWORD moduleSize;          // ģ���С
    HMODULE hModule;           // ģ����
    ModuleInfo() {}
    ModuleInfo(const std::wstring wstrModuleName, const std::wstring wstrFullPath,
        BYTE* pBaseAddress, DWORD dwModuleSize, HMODULE hModuleIn) : 
        moduleName(wstrModuleName), fullPath(wstrFullPath), baseAddress(pBaseAddress),
        moduleSize(dwModuleSize), hModule(hModuleIn) { }
};

class ProcessUtils
{
public:
    // ö��ϵͳ����
    static std::vector<DWORD> EnumProcess();

    // ��ȡ������ϸ��Ϣ
    static ProcessInfo GetProcessInfo(DWORD pid);

    // �������Ƿ�����
    static bool IsProcessRunning(const std::wstring& processName);

    // ��ȡ����ģ���б�
    static std::vector<ModuleInfo> GetProcessModules(DWORD pid);

    // ��������Ȩ��
    static bool ElevatePrivileges(const std::wstring& privilege = L"SeDebugPrivilege");

    // ��ֹ����
    static bool TerminateProcessById(DWORD pid);

    // ��ȡ��ǰ����ID
    static DWORD GetCurrentProcessId();

    // ��ȡ������ID
    static DWORD GetParentProcessId(DWORD pid);

    // ��������
    static bool CreateProcessWithArgs(const std::wstring& exePath, const std::wstring& args);
};

