#pragma once
#include <vector>
#include <string>
#include <Windows.h>

// 暂时只获取了processId 和 processName
struct ProcessInfo {
    DWORD processId;           // 进程ID
    std::wstring processName;  // 进程名称
    std::wstring fullPath;     // 进程完整路径
    DWORD parentProcessId;     // 父进程ID
    DWORD threadCount;         // 线程数量
    DWORD priorityClass;       // 优先级
    bool is64Bit;              // 是否为64位进程
    FILETIME creationTime;     // 创建时间
};

struct ModuleInfo {
    std::wstring moduleName;   // 模块名称
    std::wstring fullPath;     // 模块完整路径
    BYTE* baseAddress;         // 模块基址
    DWORD moduleSize;          // 模块大小
    HMODULE hModule;           // 模块句柄
    ModuleInfo() {}
    ModuleInfo(const std::wstring wstrModuleName, const std::wstring wstrFullPath,
        BYTE* pBaseAddress, DWORD dwModuleSize, HMODULE hModuleIn) : 
        moduleName(wstrModuleName), fullPath(wstrFullPath), baseAddress(pBaseAddress),
        moduleSize(dwModuleSize), hModule(hModuleIn) { }
};

class ProcessUtils
{
public:
    // 枚举系统进程
    static std::vector<DWORD> EnumProcess();

    // 获取进程详细信息
    static ProcessInfo GetProcessInfo(DWORD pid);

    // 使用进程名称获取进程id
    static DWORD GetProcessIdByName(const std::wstring& processName);

    // 检查进程是否运行
    static bool IsProcessRunning(const std::wstring& processName);

    // 获取进程模块列表
    static std::vector<ModuleInfo> GetProcessModules(DWORD pid);

    // 提升进程权限
    static bool ElevatePrivileges(const std::wstring& privilege = SE_DEBUG_NAME);

    // 终止进程
    static bool TerminateProcessById(DWORD pid);

    // 获取当前进程ID
    static DWORD GetCurrentProcessId();

    // 获取父进程ID
    static DWORD GetParentProcessId(DWORD pid);

    // 创建进程
    static bool CreateProcessWithArgs(const std::wstring& exePath, const std::wstring& args);

    // 判断进程是否为64位
    static int IsProcess64Bit(DWORD pid);
private:
    /*
     * 以下代码为规范性代码，工具类应避免显式生成对象，使用::的形式进行调用
     * 删除拷贝构造函数及拷贝赋值函数避免友元函数或成员函数进行拷贝操作
     */
    ProcessUtils() {} // 私有构造函数，防止外部调用构造
    ~ProcessUtils() {} // 私有析构函数，防止外部调用析构
    ProcessUtils(const ProcessUtils&) = delete; // 删除拷贝构造函数
    ProcessUtils& operator=(const ProcessUtils&) = delete; // 删除拷贝赋值操作
};

