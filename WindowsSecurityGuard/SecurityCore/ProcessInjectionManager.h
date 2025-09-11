#pragma once

#include <vector>
#include <string>

#include <Windows.h>

#include "IInjectionStrategy.h"

enum class InjectionMethod {
    CreateRemoteThread,    // 使用CreateRemoteThread API
    QueueUserAPC,         // 使用QueueUserAPC API
    SetWindowsHookEx,     // 使用SetWindowsHookEx API
    ThreadHijacking       // 线程劫持
};

// 进程注入管理器类，提供对外接口，只有这里的接口可以直接给外部业务调用
struct InjectionResult {
    bool Success;                  // 注入是否成功
    std::wstring ErrorMessage;    // 错误信息
    DWORD ErrorCode;              // 错误代码
    DWORD InjectedModuleBase;     // 注入模块基址
    InjectionResult() : Success(false), ErrorCode(0), InjectedModuleBase(0) {}
};

class ProcessInjectionManager {
public:
    // 单例访问点
    static ProcessInjectionManager& GetInstance();

    // 注入操作
    InjectionResult InjectDll(DWORD processId, const std::wstring& dllPath, InjectionMethod method = InjectionMethod::CreateRemoteThread);
    InjectionResult EjectDll(DWORD processId, const std::wstring& dllPath);
    
    /* 预留接口，远程注入别的主机 */
    //InjectionResult InjectDll(const IInjectionTarget& target, const std::wstring& dllPath, InjectionMethod method = InjectionMethod::CreateRemoteThread);
    //InjectionResult EjectDll(const IInjectionTarget& target, const std::wstring& dllPath);

private:
    // 创建注入策略
    bool CreateInjectStrategy(InjectionMethod method);

    IInjectionStrategy* GetStrategy(InjectionMethod method);

private:
    // 私有构造和析构函数（单例模式）
    ProcessInjectionManager();
    ~ProcessInjectionManager();

    // 禁用拷贝构造和赋值操作
    ProcessInjectionManager(const ProcessInjectionManager&) = delete;
    ProcessInjectionManager& operator=(const ProcessInjectionManager&) = delete;
};

