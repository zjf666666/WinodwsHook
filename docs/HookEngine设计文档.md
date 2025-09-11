# HookEngine 设计文档

## 1. 功能与用途

HookEngine 是 Windows 安全防护系统的核心组件，提供统一的 Hook 技术实现和管理框架，为系统提供函数拦截、行为监控和安全防护能力。

### 核心功能：

#### Hook 技术实现
- **功能**：
  - Inline Hook 实现（修改函数前几个字节）
  - IAT Hook 实现（修改导入地址表）
  - EAT Hook 实现（修改导出地址表）
  - 消息 Hook 实现（SetWindowsHookEx）
  - VEH/SEH Hook 实现（异常处理钩子）
- **用途**：提供多种 Hook 技术实现，适应不同场景需求

#### Hook 管理框架
- **功能**：
  - Hook 安装与卸载
  - Hook 状态管理
  - Hook 冲突检测与解决
  - 多线程安全保障
- **用途**：统一管理系统中的所有 Hook，确保稳定性和安全性

#### 远程注入支持
- **功能**：
  - CreateRemoteThread 注入
  - QueueUserAPC 注入
  - SetWindowsHookEx 注入
  - 反注入检测与防护
- **用途**：支持跨进程 Hook 实现，扩展系统防护范围

#### 内存操作支持
- **功能**：
  - 内存读写操作
  - 内存保护属性修改
  - 内存分配与释放
  - 内存模式搜索
- **用途**：为 Hook 操作提供底层内存支持

## 2. 设计模式及原因

### 工厂模式 (Factory Pattern)

**应用场景**：创建不同类型的 Hook 实现（Inline Hook、IAT Hook 等）

**实现方式**：
```cpp
class HookFactory {
public:
    static IHook* CreateHook(HookType type, const HookParams& params);
};
```

**选择原因**：
1. **封装创建逻辑**：隐藏不同 Hook 类型的创建细节，客户端代码只需关注使用
2. **统一接口**：提供统一的创建接口，简化客户端代码
3. **扩展性**：可以方便地添加新的 Hook 类型，无需修改现有代码
4. **依赖抽象**：客户端依赖于抽象接口，而非具体实现，符合依赖倒置原则

### 策略模式 (Strategy Pattern)

**应用场景**：不同 Hook 技术的具体实现

**实现方式**：
```cpp
class IHook {
public:
    virtual bool Install() = 0;
    virtual bool Uninstall() = 0;
    virtual bool IsInstalled() = 0;
};

class InlineHook : public IHook { /* 实现 */ };
class IATHook : public IHook { /* 实现 */ };
```

**选择原因**：
1. **算法封装**：将不同的 Hook 技术实现封装在各自的类中
2. **运行时选择**：允许在运行时选择不同的 Hook 策略
3. **消除条件判断**：避免使用大量的条件语句判断不同 Hook 类型
4. **开闭原则**：新增 Hook 技术只需添加新的策略类，无需修改现有代码

### 单例模式 (Singleton Pattern)

**应用场景**：HookEngine 核心管理器

**实现方式**：
```cpp
class HookEngine {
public:
    static HookEngine& GetInstance();
    // 其他方法...
private:
    HookEngine();
    ~HookEngine();
    // 禁用拷贝构造和赋值操作
};
```

**选择原因**：
1. **全局访问点**：提供对 HookEngine 的全局访问点
2. **资源共享**：确保系统中只有一个 HookEngine 实例，避免资源冲突
3. **状态统一**：集中管理所有 Hook，确保状态一致性
4. **延迟初始化**：支持在首次使用时才创建实例，优化资源使用

### 观察者模式 (Observer Pattern)

**应用场景**：Hook 事件通知

**实现方式**：
```cpp
class IHookObserver {
public:
    virtual void OnHookInstalled(const HookInfo& info) = 0;
    virtual void OnHookUninstalled(const HookInfo& info) = 0;
    virtual void OnHookCalled(const HookCallInfo& info) = 0;
};

class HookEngine {
public:
    void AddObserver(IHookObserver* observer);
    void RemoveObserver(IHookObserver* observer);
    // 其他方法...
private:
    std::vector<IHookObserver*> m_observers;
};
```

**选择原因**：
1. **解耦通知**：Hook 事件发生时，无需知道谁对此感兴趣，只需通知所有观察者
2. **扩展监控**：可以方便地添加新的监控组件，如日志记录、行为分析等
3. **一对多依赖**：支持多个组件同时监控 Hook 事件
4. **开闭原则**：添加新的观察者不需要修改 HookEngine 代码

### 命令模式 (Command Pattern)

**应用场景**：Hook 操作的执行和撤销

**实现方式**：
```cpp
class IHookCommand {
public:
    virtual bool Execute() = 0;
    virtual bool Undo() = 0;
};

class InstallHookCommand : public IHookCommand { /* 实现 */ };
class UninstallHookCommand : public IHookCommand { /* 实现 */ };
```

**选择原因**：
1. **操作封装**：将 Hook 操作封装为对象，支持参数化和队列化
2. **撤销支持**：提供统一的撤销机制，增强系统稳定性
3. **事务支持**：可以将多个 Hook 操作组合为事务，要么全部成功，要么全部失败
4. **日志记录**：便于记录所有 Hook 操作，支持审计和问题排查

### 装饰器模式 (Decorator Pattern)

**应用场景**：扩展 Hook 功能

**实现方式**：
```cpp
class HookDecorator : public IHook {
public:
    HookDecorator(IHook* hook) : m_hook(hook) {}
    virtual bool Install() override { return m_hook->Install(); }
    virtual bool Uninstall() override { return m_hook->Uninstall(); }
    virtual bool IsInstalled() override { return m_hook->IsInstalled(); }
protected:
    IHook* m_hook;
};

class LoggingHookDecorator : public HookDecorator { /* 实现 */ };
class ThreadSafeHookDecorator : public HookDecorator { /* 实现 */ };
```

**选择原因**：
1. **动态扩展**：在运行时动态添加功能，如日志记录、线程安全等
2. **组合灵活**：可以灵活组合多个装饰器，实现功能叠加
3. **单一职责**：每个装饰器只关注一个功能点，符合单一职责原则
4. **避免继承爆炸**：通过组合而非继承实现功能扩展，避免类层次结构复杂化

## 3. 核心接口设计

### IHook 接口

```cpp
class IHook {
public:
    virtual ~IHook() {}
    
    // 安装 Hook
    virtual bool Install() = 0;
    
    // 卸载 Hook
    virtual bool Uninstall() = 0;
    
    // 检查 Hook 是否已安装
    virtual bool IsInstalled() = 0;
    
    // 获取原始函数
    template<typename T>
    T GetOriginalFunction() {
        return reinterpret_cast<T>(GetOriginalFunctionAddress());
    }
    
protected:
    // 获取原始函数地址（供模板方法使用）
    virtual void* GetOriginalFunctionAddress() = 0;
};
```

### HookEngine 核心类

```cpp
class HookEngine {
public:
    // 单例访问点
    static HookEngine& GetInstance();
    
    // Hook 管理
    bool InstallHook(const std::wstring& hookType, const std::wstring& targetModule, 
                    const std::string& targetFunction, void* hookFunction, 
                    HookId& outHookId);
    bool UninstallHook(HookId hookId);
    bool EnableHook(HookId hookId, bool enable);
    
    // Hook 查询
    bool GetHookInfo(HookId hookId, HookInfo& outInfo);
    std::vector<HookId> FindHooks(const HookQuery& query);
    
    // 观察者管理
    void AddObserver(IHookObserver* observer);
    void RemoveObserver(IHookObserver* observer);
    
    // 全局设置
    void SetGlobalOptions(const HookOptions& options);
    HookOptions GetGlobalOptions() const;
    
private:
    // 私有构造和析构函数（单例模式）
    HookEngine();
    ~HookEngine();
    
    // 禁用拷贝构造和赋值操作
    HookEngine(const HookEngine&) = delete;
    HookEngine& operator=(const HookEngine&) = delete;
    
    // 内部实现细节
    // ...
};
```

## 4. 组件间关系

```
┌─────────────────────────────────────────────────────────────┐
│                      HookEngine                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │ HookFactory │ │ HookManager │ │ HookRegistry│           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
│                                                             │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │ InjectionMgr│ │ MemoryUtils │ │ HookObserver│           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│                     Hook 实现                               │
│                                                             │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │ InlineHook  │ │  IAT Hook   │ │  EAT Hook   │           │
│  └─────────────┘ └─────────────┘ └─────────────┘           │
│                                                             │
│  ┌─────────────┐ ┌─────────────┐                           │
│  │ MessageHook │ │  VEH Hook   │                           │
│  └─────────────┘ └─────────────┘                           │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## 5. 设计原则遵循

### SOLID 原则

1. **单一职责原则 (SRP)**
   - 每个类只负责一个功能领域，如 InlineHook 只负责内联钩子实现
   - HookManager 专注于 Hook 生命周期管理，不涉及具体实现

2. **开闭原则 (OCP)**
   - 通过接口和抽象类设计，对扩展开放，对修改封闭
   - 添加新的 Hook 类型不需要修改现有代码

3. **里氏替换原则 (LSP)**
   - 所有 Hook 实现类可以替换 IHook 接口的位置
   - 客户端代码不需要知道具体使用的是哪种 Hook 实现

4. **接口隔离原则 (ISP)**
   - 接口设计精简，避免客户端依赖不需要的方法
   - 根据功能划分多个小接口，如 IHook、IHookObserver 等

5. **依赖倒置原则 (DIP)**
   - 高层模块依赖抽象接口，而非具体实现
   - 通过工厂模式和依赖注入实现解耦

### 其他设计原则

1. **组合优于继承**
   - 使用装饰器模式而非继承来扩展功能
   - 通过组合不同的 Hook 策略实现复杂功能

2. **最小知识原则 (迪米特法则)**
   - 组件之间通过明确定义的接口交互
   - 减少组件间的直接依赖，降低耦合度

3. **高内聚低耦合**
   - 相关功能集中在同一个类中
   - 通过接口和观察者模式降低组件间耦合

## 6. 扩展性设计

1. **插件架构**
   - 支持通过插件方式添加新的 Hook 类型
   - 定义清晰的插件接口和生命周期管理

2. **配置驱动**
   - 通过配置文件定义 Hook 规则和行为
   - 支持运行时动态加载和更新配置

3. **事件驱动**
   - 基于观察者模式的事件通知机制
   - 支持自定义事件处理器扩展系统功能

4. **中间件支持**
   - 允许在 Hook 处理流程中插入自定义中间件
   - 支持前置处理、后置处理和异常处理

## 7. 安全性设计

1. **自我保护**
   - 防止 HookEngine 自身被 Hook 或篡改
   - 关键数据加密存储，防止内存扫描

2. **权限控制**
   - 验证调用者权限，防止未授权使用
   - 支持不同级别的操作权限控制

3. **异常恢复**
   - 完善的异常处理机制，确保系统稳定性
   - 支持事务回滚，防止部分操作失败导致系统不一致

4. **审计日志**
   - 记录所有 Hook 操作，支持安全审计
   - 提供详细的错误信息和诊断数据

## 8. 性能优化

1. **资源池化**
   - 重用 Hook 对象和内存资源，减少分配开销
   - 批量处理 Hook 操作，减少系统调用

2. **延迟加载**
   - 按需创建和初始化组件，减少启动开销
   - 支持热插拔，动态加载和卸载功能模块

3. **并发优化**
   - 细粒度锁设计，减少线程竞争
   - 无锁数据结构，提高并发性能

4. **内存布局优化**
   - 考虑缓存友好的数据结构设计
   - 减少内存碎片和指针跳转

## 9. 业务应用场景

### 客户端使用流程

从客户端角度看，使用HookEngine的基本流程是：

```cpp
// 1. 获取HookEngine单例
HookEngine& engine = HookEngine::GetInstance();

// 2. 安装Hook
HookId hookId;
engine.InstallHook(L"InlineHook", L"user32.dll", "MessageBoxA", MyHookFunction, hookId);

// 3. 使用系统，Hook会自动生效
// ...

// 4. 卸载Hook
engine.UninstallHook(hookId);
```

看起来很简单，但这背后隐藏了复杂的业务逻辑，这就是为什么需要多种设计模式协同工作。

### 各设计模式的业务价值

#### 工厂模式的业务价值

当你调用`InstallHook`时，内部会使用工厂模式：

```cpp
// HookEngine内部实现
bool HookEngine::InstallHook(...) {
    // 使用工厂创建具体的Hook实现
    IHook* hook = HookFactory::CreateHook(hookType, params);
    // 注册并安装Hook
    hook->Install();
    // ...
}
```

**业务价值**：
- 客户端不需要了解不同Hook技术的实现细节
- 可以根据目标函数特性自动选择最合适的Hook技术
- 支持动态添加新的Hook技术而不影响现有代码

#### 策略模式的业务价值

**业务场景**：
- 不同API需要不同的Hook技术：有些API适合用Inline Hook，有些适合IAT Hook
- 不同进程环境需要不同策略：32位/64位、不同Windows版本
- 安全软件需要根据目标程序特性动态调整Hook策略

例如，当你需要监控文件操作时：
```cpp
// 对于普通应用，使用IAT Hook更稳定
engine.InstallHook(L"IATHook", L"kernel32.dll", "CreateFileW", MyCreateFileHook, hookId1);

// 对于加壳或保护的应用，可能需要Inline Hook
engine.InstallHook(L"InlineHook", L"kernel32.dll", "CreateFileW", MyCreateFileHook, hookId2);
```

**业务价值**：
- 可以根据不同场景选择最合适的Hook技术
- 支持运行时动态切换Hook策略
- 同一个API可以应用不同的Hook技术，提高成功率

#### 观察者模式的业务价值

**业务场景**：
- 安全软件需要记录所有Hook事件用于审计
- 行为分析引擎需要实时获取Hook调用信息
- 多个安全模块需要协同工作，共享Hook信息

例如：
```cpp
// 日志观察者
class LogObserver : public IHookObserver {
    void OnHookCalled(const HookCallInfo& info) override {
        Logger::Log("Hook调用: %s", info.functionName.c_str());
    }
};

// 行为分析观察者
class BehaviorAnalyzer : public IHookObserver {
    void OnHookCalled(const HookCallInfo& info) override {
        AnalyzeCall(info); // 分析调用行为是否可疑
    }
};

// 注册观察者
LogObserver logObs;
BehaviorAnalyzer analyzer;
engine.AddObserver(&logObs);
engine.AddObserver(&analyzer);
```

**业务价值**：
- 实现Hook事件的多方通知，无需修改Hook核心代码
- 支持动态添加/移除监控组件
- 不同安全模块可以独立响应Hook事件
- 实现复杂的行为分析和威胁检测

#### 单例模式的业务价值

**业务场景**：
- 全局Hook状态管理，确保一致性
- 防止重复Hook导致的系统不稳定
- 集中管理所有Hook资源

**业务价值**：
- 提供全局访问点，简化客户端代码
- 确保系统中只有一个Hook管理器，避免冲突
- 集中管理Hook生命周期，提高系统稳定性

#### 命令模式的业务价值

**业务场景**：
- 需要支持Hook操作的撤销/重做
- 需要将多个Hook操作作为一个事务执行
- 需要延迟执行或队列化Hook操作

例如，当检测到威胁时，需要批量安装多个Hook：
```cpp
// 创建命令组
HookCommandGroup group;
group.AddCommand(new InstallHookCommand(engine, "CreateFileW", MyFileHook));
group.AddCommand(new InstallHookCommand(engine, "RegSetValueEx", MyRegHook));
group.AddCommand(new InstallHookCommand(engine, "connect", MyNetworkHook));

// 作为一个事务执行
if (!group.Execute()) {
    // 如果任何一个失败，全部回滚
    group.Undo();
}
```

**业务价值**：
- 支持Hook操作的事务性，确保系统一致性
- 提供撤销机制，增强系统稳定性
- 支持操作历史记录，便于问题排查

#### 装饰器模式的业务价值

**业务场景**：
- 需要为Hook添加额外功能，如日志记录、性能监控
- 需要动态组合多种功能

例如：
```cpp
// 创建基本Hook
IHook* baseHook = HookFactory::CreateHook(HookType::Inline, params);

// 添加线程安全装饰
IHook* threadSafeHook = new ThreadSafeHookDecorator(baseHook);

// 添加日志装饰
IHook* loggingHook = new LoggingHookDecorator(threadSafeHook);

// 使用装饰后的Hook
loggingHook->Install();
```

**业务价值**：
- 动态扩展Hook功能，无需修改原有代码
- 灵活组合多种功能，适应不同需求
- 遵循单一职责原则，每个装饰器只关注一个功能点

### 实际业务调用流程

当你调用`engine.InstallHook(...)`时，内部发生的完整流程是：

1. **参数验证**：验证传入参数的有效性
2. **工厂创建**：使用工厂模式创建具体的Hook实现
   ```cpp
   IHook* hook = HookFactory::CreateHook(hookType, params);
   ```
3. **策略应用**：根据目标函数特性选择最合适的Hook策略
4. **Hook安装**：调用具体Hook实现的Install方法
   ```cpp
   hook->Install();
   ```
5. **注册表登记**：将Hook信息登记到内部注册表
   ```cpp
   m_registry.RegisterHook(hookId, hook);
   ```
6. **观察者通知**：通知所有观察者Hook已安装
   ```cpp
   NotifyObservers(HookEvent::Installed, hookInfo);
   ```