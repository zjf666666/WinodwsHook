# HookEngine 简化设计文档

## 1. 设计概述

HookEngine 是 Windows 安全防护系统的核心组件，提供统一的 Hook 技术实现和管理框架。本文档描述了简化版 HookEngine 的设计，仅保留工厂模式、策略模式、观察者模式和单例模式的实现。

## 2. 核心类设计

### 2.1 文件结构

```
HookEngine/
├── IHook.h                 // Hook 接口定义
├── HookTypes.h             // Hook 类型和常量定义
├── InlineHook.h            // Inline Hook 实现
├── InlineHook.cpp
├── IATHook.h               // IAT Hook 实现
├── IATHook.cpp
├── HookFactory.h           // Hook 工厂
├── HookFactory.cpp
├── IHookObserver.h         // Hook 观察者接口
├── HookEngine.h            // 核心引擎（单例）
└── HookEngine.cpp
```

### 2.2 类图

```
┌─────────────────┐      ┌─────────────────┐
│   HookEngine    │<>────│  IHookObserver  │
└─────────────────┘      └─────────────────┘
        △                        △
        │                        │
        │                 ┌──────┴──────┐
        │                 │ LogObserver │
        │                 └─────────────┘
        │
        │uses
        │
┌─────────────────┐      ┌─────────────────┐
│   HookFactory   │─────>│      IHook      │
└─────────────────┘      └─────────────────┘
                                 △
                                 │
                      ┌──────────┴──────────┐
                      │                      │
               ┌──────┴─────┐        ┌──────┴─────┐
               │ InlineHook │        │  IATHook   │
               └────────────┘        └────────────┘
```

## 3. 接口设计

### 3.1 IHook 接口

```cpp
// IHook.h
#pragma once
#include <string>

/**
 * @brief Hook 接口类，定义所有 Hook 实现的通用接口
 */
class IHook {
public:
    virtual ~IHook() = default;
    
    /**
     * @brief 安装 Hook
     * @return 安装是否成功
     * @note 由 HookEngine 调用
     */
    virtual bool Install() = 0;
    
    /**
     * @brief 卸载 Hook
     * @return 卸载是否成功
     * @note 由 HookEngine 调用
     */
    virtual bool Uninstall() = 0;
    
    /**
     * @brief 检查 Hook 是否已安装
     * @return Hook 是否已安装
     * @note 由 HookEngine 调用
     */
    virtual bool IsInstalled() const = 0;
    
    /**
     * @brief 获取原始函数
     * @return 原始函数指针
     * @note 由用户代码调用
     */
    template<typename T>
    T GetOriginalFunction() {
        return reinterpret_cast<T>(GetOriginalFunctionAddress());
    }
    
    /**
     * @brief 获取目标模块名
     * @return 目标模块名
     * @note 由 HookEngine 调用
     */
    virtual const std::wstring& GetTargetModule() const = 0;
    
    /**
     * @brief 获取目标函数名
     * @return 目标函数名
     * @note 由 HookEngine 调用
     */
    virtual const std::string& GetTargetFunction() const = 0;
    
    /**
     * @brief 获取 Hook 类型
     * @return Hook 类型
     * @note 由 HookEngine 调用
     */
    virtual const std::wstring& GetHookType() const = 0;

protected:
    /**
     * @brief 获取原始函数地址
     * @return 原始函数地址
     * @note 由 GetOriginalFunction 模板方法调用
     */
    virtual void* GetOriginalFunctionAddress() const = 0;
};
```

### 3.2 Hook 类型定义

```cpp
// HookTypes.h
#pragma once
#include <string>

/**
 * @brief Hook 类型枚举
 */
enum class HookType {
    Inline,     ///< Inline Hook
    IAT,        ///< Import Address Table Hook
};

/**
 * @brief Hook ID 类型
 */
typedef unsigned long HookId;

/**
 * @brief Hook 信息结构体
 */
struct HookInfo {
    HookId id;                  ///< Hook ID
    std::wstring hookType;      ///< Hook 类型
    std::wstring targetModule;  ///< 目标模块
    std::string targetFunction; ///< 目标函数
    void* hookFunction;         ///< Hook 函数
    bool isEnabled;             ///< 是否启用
};

/**
 * @brief Hook 调用信息结构体
 */
struct HookCallInfo {
    HookId hookId;              ///< Hook ID
    std::string functionName;   ///< 函数名
    void* originalFunction;     ///< 原始函数
    void* hookFunction;         ///< Hook 函数
    void* returnAddress;        ///< 返回地址
    void* context;              ///< 上下文信息
};
```

### 3.3 具体 Hook 实现

#### 3.3.1 InlineHook

```cpp
// InlineHook.h
#pragma once
#include "IHook.h"

/**
 * @brief Inline Hook 实现类
 */
class InlineHook : public IHook {
public:
    /**
     * @brief 构造函数
     * @param targetModule 目标模块名
     * @param targetFunction 目标函数名
     * @param hookFunction Hook 函数指针
     * @note 由 HookFactory 调用
     */
    InlineHook(const std::wstring& targetModule, const std::string& targetFunction, void* hookFunction);
    
    ~InlineHook() override;
    
    // IHook 接口实现
    bool Install() override;
    bool Uninstall() override;
    bool IsInstalled() const override;
    const std::wstring& GetTargetModule() const override;
    const std::string& GetTargetFunction() const override;
    const std::wstring& GetHookType() const override;
    
protected:
    void* GetOriginalFunctionAddress() const override;
    
private:
    std::wstring m_targetModule;    ///< 目标模块名
    std::string m_targetFunction;   ///< 目标函数名
    void* m_hookFunction;           ///< Hook 函数指针
    void* m_originalFunction;       ///< 原始函数指针
    bool m_installed;               ///< 是否已安装
    unsigned char m_originalBytes[16]; ///< 原始字节
};
```

#### 3.3.2 IATHook

```cpp
// IATHook.h
#pragma once
#include "IHook.h"

/**
 * @brief IAT Hook 实现类
 */
class IATHook : public IHook {
public:
    /**
     * @brief 构造函数
     * @param targetModule 目标模块名
     * @param targetFunction 目标函数名
     * @param hookFunction Hook 函数指针
     * @note 由 HookFactory 调用
     */
    IATHook(const std::wstring& targetModule, const std::string& targetFunction, void* hookFunction);
    
    ~IATHook() override;
    
    // IHook 接口实现
    bool Install() override;
    bool Uninstall() override;
    bool IsInstalled() const override;
    const std::wstring& GetTargetModule() const override;
    const std::string& GetTargetFunction() const override;
    const std::wstring& GetHookType() const override;
    
protected:
    void* GetOriginalFunctionAddress() const override;
    
private:
    std::wstring m_targetModule;    ///< 目标模块名
    std::string m_targetFunction;   ///< 目标函数名
    void* m_hookFunction;           ///< Hook 函数指针
    void* m_originalFunction;       ///< 原始函数指针
    void** m_iatEntry;              ///< IAT 表项指针
    bool m_installed;               ///< 是否已安装
};
```

### 3.4 Hook 工厂

```cpp
// HookFactory.h
#pragma once
#include "IHook.h"
#include "HookTypes.h"

/**
 * @brief Hook 工厂类，负责创建不同类型的 Hook 对象
 */
class HookFactory {
public:
    /**
     * @brief 创建 Hook 对象
     * @param hookType Hook 类型
     * @param targetModule 目标模块名
     * @param targetFunction 目标函数名
     * @param hookFunction Hook 函数指针
     * @return Hook 对象指针，失败返回 nullptr
     * @note 由 HookEngine 调用
     */
    static IHook* CreateHook(
        const std::wstring& hookType,
        const std::wstring& targetModule,
        const std::string& targetFunction,
        void* hookFunction);
};
```

### 3.5 Hook 观察者接口

```cpp
// IHookObserver.h
#pragma once
#include "HookTypes.h"

/**
 * @brief Hook 事件类型
 */
enum class HookEvent {
    Installed,      ///< Hook 已安装
    Uninstalled,    ///< Hook 已卸载
    Called,         ///< Hook 被调用
    Error           ///< 发生错误
};

/**
 * @brief Hook 观察者接口
 */
class IHookObserver {
public:
    virtual ~IHookObserver() = default;
    
    /**
     * @brief Hook 事件通知
     * @param event 事件类型
     * @param hookInfo Hook 信息
     * @note 由 HookEngine 调用
     */
    virtual void OnHookEvent(HookEvent event, const HookInfo& hookInfo) = 0;
    
    /**
     * @brief Hook 调用通知
     * @param callInfo 调用信息
     * @note 由 HookEngine 调用
     */
    virtual void OnHookCalled(const HookCallInfo& callInfo) = 0;
};
```

### 3.6 HookEngine 核心类

```cpp
// HookEngine.h
#pragma once
#include "IHook.h"
#include "IHookObserver.h"
#include <vector>
#include <map>
#include <mutex>

/**
 * @brief Hook 引擎核心类，单例模式
 */
class HookEngine {
public:
    /**
     * @brief 获取单例实例
     * @return HookEngine 单例引用
     * @note 由客户端代码调用
     */
    static HookEngine& GetInstance();
    
    /**
     * @brief 安装 Hook
     * @param hookType Hook 类型
     * @param targetModule 目标模块名
     * @param targetFunction 目标函数名
     * @param hookFunction Hook 函数指针
     * @param outHookId 输出参数，安装成功后返回 Hook ID
     * @return 安装是否成功
     * @note 由客户端代码调用
     */
    bool InstallHook(
        const std::wstring& hookType,
        const std::wstring& targetModule,
        const std::string& targetFunction,
        void* hookFunction,
        HookId& outHookId);
    
    /**
     * @brief 卸载 Hook
     * @param hookId Hook ID
     * @return 卸载是否成功
     * @note 由客户端代码调用
     */
    bool UninstallHook(HookId hookId);
    
    /**
     * @brief 启用/禁用 Hook
     * @param hookId Hook ID
     * @param enable 是否启用
     * @return 操作是否成功
     * @note 由客户端代码调用
     */
    bool EnableHook(HookId hookId, bool enable);
    
    /**
     * @brief 获取 Hook 信息
     * @param hookId Hook ID
     * @param outInfo 输出参数，获取成功后返回 Hook 信息
     * @return 获取是否成功
     * @note 由客户端代码调用
     */
    bool GetHookInfo(HookId hookId, HookInfo& outInfo);
    
    /**
     * @brief 添加观察者
     * @param observer 观察者指针
     * @note 由客户端代码调用
     */
    void AddObserver(IHookObserver* observer);
    
    /**
     * @brief 移除观察者
     * @param observer 观察者指针
     * @note 由客户端代码调用
     */
    void RemoveObserver(IHookObserver* observer);
    
    /**
     * @brief 通知 Hook 调用事件
     * @param hookId Hook ID
     * @param callInfo 调用信息
     * @note 由 Hook 函数调用
     */
    void NotifyHookCalled(HookId hookId, const HookCallInfo& callInfo);
    
private:
    // 私有构造和析构函数（单例模式）
    HookEngine();
    ~HookEngine();
    
    // 禁用拷贝构造和赋值操作
    HookEngine(const HookEngine&) = delete;
    HookEngine& operator=(const HookEngine&) = delete;
    
    /**
     * @brief 通知观察者
     * @param event 事件类型
     * @param hookInfo Hook 信息
     */
    void NotifyObservers(HookEvent event, const HookInfo& hookInfo);
    
    /**
     * @brief 生成新的 Hook ID
     * @return 新的 Hook ID
     */
    HookId GenerateHookId();
    
private:
    std::map<HookId, IHook*> m_hooks;                ///< Hook 映射表
    std::map<HookId, HookInfo> m_hookInfos;          ///< Hook 信息映射表
    std::vector<IHookObserver*> m_observers;         ///< 观察者列表
    std::mutex m_mutex;                              ///< 互斥锁
    HookId m_nextHookId;                            ///< 下一个 Hook ID
};
```

## 4. 使用示例

```cpp
// 客户端代码示例

// 1. 定义 Hook 函数
void* g_OriginalMessageBoxA = nullptr;

int WINAPI MyMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    // 获取 HookEngine 单例
    HookEngine& engine = HookEngine::GetInstance();
    
    // 记录调用信息
    HookCallInfo callInfo;
    callInfo.functionName = "MessageBoxA";
    callInfo.originalFunction = g_OriginalMessageBoxA;
    callInfo.hookFunction = (void*)MyMessageBoxA;
    callInfo.returnAddress = _ReturnAddress();
    
    // 通知 Hook 调用事件
    engine.NotifyHookCalled(1, callInfo); // 假设 Hook ID 为 1
    
    // 修改参数
    std::string newText = std::string(lpText) + " [Hooked]";
    
    // 调用原始函数
    typedef int (WINAPI *MessageBoxA_t)(HWND, LPCSTR, LPCSTR, UINT);
    MessageBoxA_t originalFunc = (MessageBoxA_t)g_OriginalMessageBoxA;
    return originalFunc(hWnd, newText.c_str(), lpCaption, uType);
}

// 2. 实现观察者
class MyHookObserver : public IHookObserver {
public:
    void OnHookEvent(HookEvent event, const HookInfo& hookInfo) override {
        switch (event) {
            case HookEvent::Installed:
                std::cout << "Hook installed: " << hookInfo.targetFunction << std::endl;
                break;
            case HookEvent::Uninstalled:
                std::cout << "Hook uninstalled: " << hookInfo.targetFunction << std::endl;
                break;
            case HookEvent::Error:
                std::cout << "Hook error: " << hookInfo.targetFunction << std::endl;
                break;
        }
    }
    
    void OnHookCalled(const HookCallInfo& callInfo) override {
        std::cout << "Hook called: " << callInfo.functionName << std::endl;
    }
};

// 3. 使用 HookEngine
void UseHookEngine() {
    // 获取 HookEngine 单例
    HookEngine& engine = HookEngine::GetInstance();
    
    // 添加观察者
    MyHookObserver observer;
    engine.AddObserver(&observer);
    
    // 安装 Hook
    HookId hookId;
    bool success = engine.InstallHook(
        L"InlineHook",
        L"user32.dll",
        "MessageBoxA",
        (void*)MyMessageBoxA,
        hookId);
    
    if (success) {
        // 保存原始函数指针
        HookInfo hookInfo;
        engine.GetHookInfo(hookId, hookInfo);
        g_OriginalMessageBoxA = ((InlineHook*)hookInfo.hookFunction)->GetOriginalFunction<void*>();
        
        // 使用被 Hook 的函数
        MessageBoxA(NULL, "Test", "Title", MB_OK);
        
        // 卸载 Hook
        engine.UninstallHook(hookId);
    }
    
    // 移除观察者
    engine.RemoveObserver(&observer);
}
```

## 5. 实现建议

1. **分阶段实现**：
   - 第一阶段：实现基本接口和 InlineHook
   - 第二阶段：添加 IATHook 支持
   - 第三阶段：实现观察者模式

2. **错误处理**：
   - 所有公共方法应返回错误码或布尔值
   - 使用观察者模式通知错误事件

3. **线程安全**：
   - 使用互斥锁保护共享资源
   - 确保 Hook 安装/卸载操作的原子性

4. **内存管理**：
   - 明确 Hook 对象的所有权（由 HookEngine 管理）
   - 在 HookEngine 析构函数中释放所有 Hook 对象

5. **日志记录**：
   - 实现一个 LogObserver 用于记录 Hook 事件
   - 记录详细的错误信息和调用堆栈

## 6. 后续扩展

1. **添加更多 Hook 类型**：
   - EAT Hook
   - 消息 Hook
   - VEH/SEH Hook

2. **添加命令模式**：
   - 支持 Hook 操作的撤销/重做
   - 实现事务性操作

3. **添加装饰器模式**：
   - 为 Hook 添加额外功能，如日志记录、性能监控
   - 动态组合多种功能