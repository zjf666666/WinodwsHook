#pragma once

#include <vector>
#include <string>
#include <memory>

#include <Windows.h>

#include "IInjectionStrategy.h"

#define SIZE_INJECTION_METHOD   4

enum class InjectionMethod {
    CreateRemoteThread,    // 使用CreateRemoteThread API
    QueueUserAPC,         // 使用QueueUserAPC API
    SetWindowsHookEx,     // 使用SetWindowsHookEx API
    ThreadHijacking,       // 线程劫持
    ERROR_METHOD          // 用于判断method是否合法
};

// 这个类暂时不用
struct InjectionResult {
    DWORD ErrorCode;              // 错误代码
    DWORD InjectedModuleBase;     // 注入模块基址
    InjectionResult() : ErrorCode(0), InjectedModuleBase(0) {}
};

// 进程注入管理器类，提供对外接口，只有这里的接口可以直接给外部业务调用
class ProcessInjectionManager {
public:
    // 单例访问点
    static ProcessInjectionManager& GetInstance();

    // 注入操作
    bool InjectDll(DWORD pid, const std::wstring& dllPath, InjectionMethod method = InjectionMethod::CreateRemoteThread);
    bool EjectDll(DWORD pid, const std::wstring& dllPath);
    
    /* 预留接口，远程注入别的主机 */
    //InjectionResult InjectDll(const IInjectionTarget& target, const std::wstring& dllPath, InjectionMethod method = InjectionMethod::CreateRemoteThread);
    //InjectionResult EjectDll(const IInjectionTarget& target, const std::wstring& dllPath);

private:
    // 创建注入策略，这个函数只负责创建，判断逻辑均在外部进行
    void CreateInjectStrategy(InjectionMethod method);

    IInjectionStrategy* GetStrategy(InjectionMethod method);

private:
    // 私有构造和析构函数（单例模式）
    ProcessInjectionManager();
    ~ProcessInjectionManager();

    // 禁用拷贝构造和赋值操作
    ProcessInjectionManager(const ProcessInjectionManager&) = delete;
    ProcessInjectionManager& operator=(const ProcessInjectionManager&) = delete;

private:
    std::vector<std::unique_ptr<IInjectionStrategy>> m_vecInjectionStrategy;
};

