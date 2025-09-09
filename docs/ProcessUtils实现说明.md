# ProcessUtils 实现说明文档

## 1. 概述

`ProcessUtils` 是 Windows 安全防护系统中的核心进程工具类，提供进程操作和管理的基础功能。该组件负责系统进程的枚举、信息获取、创建和终止等操作，是实现进程监控、注入和Hook功能的重要基础设施。

## 2. 设计模式

### 静态工具类

`ProcessUtils` 类采用静态工具类模式实现，所有方法均为静态方法，不需要实例化即可使用。

**实现要点**：
1. 所有方法均为静态方法
2. 不需要维护状态
3. 提供纯功能性的API
4. 线程安全的实现

## 3. 数据结构定义

### 3.1 ProcessInfo 结构体

```cpp
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
```

**设计理由**：
1. 包含进程的基本标识信息（ID、名称、路径）
2. 包含进程的关系信息（父进程ID）
3. 包含进程的资源信息（线程数量）
4. 包含进程的属性信息（优先级、位数）
5. 包含进程的时间信息（创建时间）
6. 使用Windows API原生类型（DWORD、FILETIME）确保兼容性

### 3.2 ModuleInfo 结构体

```cpp
struct ModuleInfo {
    std::wstring moduleName;   // 模块名称
    std::wstring fullPath;     // 模块完整路径
    BYTE* baseAddress;         // 模块基址
    DWORD moduleSize;          // 模块大小
    HMODULE hModule;           // 模块句柄
};
```

**设计理由**：
1. 包含模块的基本标识信息（名称、路径）
2. 包含模块的内存信息（基址、大小）
3. 包含模块的句柄信息（便于后续操作）
4. 使用Windows API原生类型（BYTE*、DWORD、HMODULE）确保兼容性

## 4. 函数实现说明

### 4.1 EnumProcesses 枚举系统进程

```cpp
static std::vector<DWORD> EnumProcesses();
```

**使用场景**：
- 系统进程监控功能需要获取所有进程列表
- 安全扫描功能需要检查可疑进程
- 资源监控需要统计进程数量和资源占用

**实现思路**：
1. 使用 `EnumProcesses` Windows API 获取进程ID列表
2. 动态分配足够大的缓冲区以容纳所有进程ID
3. 处理可能的权限问题和错误情况
4. 将结果转换为 `std::vector<DWORD>` 返回

**注意事项**：
- 需要处理权限不足的情况
- 考虑进程列表可能在获取过程中发生变化
- 优化内存分配，避免频繁重新分配
- 考虑使用 `CreateToolhelp32Snapshot` 作为备选方案

**Windows进程遍历方式比较**：

在Windows系统中，有多种方式可以遍历进程，每种方式都有其优缺点：

1. **EnumProcesses API**
   - **优点**：
     - 简单易用，API调用直观
     - 稳定性好，兼容性强（从Windows XP到Windows 11）
     - 性能开销适中，适合大多数场景
     - 不需要特殊权限即可获取基本进程ID列表
   - **缺点**：
     - 只能获取进程ID，需要额外调用其他API获取详细信息
     - 在高并发场景下性能不如直接使用NT API
     - 需要额外处理缓冲区大小调整

2. **CreateToolhelp32Snapshot + Process32First/Process32Next**
   - **优点**：
     - 一次调用可获取更多进程信息（如进程名、父进程ID等）
     - 可以同时获取进程、线程、模块和堆信息
     - 适合需要遍历进程树的场景
   - **缺点**：
     - 在进程数量大的系统上性能略低
     - 快照机制可能导致信息不是实时的
     - 在某些安全软件干预下可能不完整

3. **WMI (Windows Management Instrumentation)**
   - **优点**：
     - 功能最强大，可获取最全面的进程信息
     - 支持远程查询和管理
     - 提供丰富的查询语言和过滤能力
   - **缺点**：
     - 性能开销最大，不适合频繁调用
     - 初始化时间长
     - 需要特定权限
     - 代码复杂度高

4. **NtQuerySystemInformation (Native API)**
   - **优点**：
     - 性能最佳，直接访问系统内核信息
     - 可以获取一些其他API无法获取的底层信息
     - 适合高性能要求的场景
   - **缺点**：
     - 非官方文档化API，可能随系统版本变化
     - 需要较高权限
     - 使用复杂，需要处理底层数据结构
     - 可能被安全软件监控或拦截

**选择EnumProcesses的理由**：

1. **平衡性**：在性能、易用性和功能之间取得了良好平衡
2. **兼容性**：作为官方文档化API，具有最佳的向前兼容性保证
3. **可维护性**：代码简洁，易于理解和维护
4. **备选方案**：实现中已考虑在特定场景下切换到其他方法的可能性
5. **分层设计**：基础层使用最稳定的API，高级功能可在此基础上扩展

### 4.2 GetProcessInfo 获取进程详细信息

```cpp
static ProcessInfo GetProcessInfo(DWORD pid);
```

**使用场景**：
- 进程详情显示功能需要获取进程的详细信息
- 进程监控功能需要记录进程的属性变化
- 安全分析需要检查进程的来源和特性

**实现思路**：
1. 使用 `OpenProcess` 获取进程句柄
2. 使用 `GetProcessImageFileName` 或 `QueryFullProcessImageName` 获取进程路径
3. 使用 `GetProcessTimes` 获取进程创建时间
4. 使用 `GetProcessPriorityClass` 获取进程优先级
5. 使用 `IsWow64Process` 检查是否为64位进程
6. 填充 `ProcessInfo` 结构并返回

**注意事项**：
- 处理进程可能已终止的情况
- 处理权限不足无法获取某些信息的情况
- 考虑使用 `CreateToolhelp32Snapshot` 和 `Process32First/Next` 作为备选方案
- 确保正确释放进程句柄

### 4.3 IsProcessRunning 检查进程是否运行

```cpp
static bool IsProcessRunning(const std::wstring& processName);
```

**使用场景**：
- 检查特定安全软件是否正在运行
- 验证系统关键服务是否活跃
- 防止应用程序重复启动

**实现思路**：
1. 枚举系统所有进程
2. 对每个进程获取其名称
3. 比较进程名称与目标名称（不区分大小写）
4. 如果找到匹配项，返回true，否则返回false

**注意事项**：
- 进程名称比较应该不区分大小写
- 考虑进程名称可能包含路径的情况
- 优化性能，避免不必要的字符串操作
- 处理权限不足的情况

### 4.4 GetProcessModules 获取进程模块列表

```cpp
static std::vector<ModuleInfo> GetProcessModules(DWORD pid);
```

**使用场景**：
- 分析进程加载的DLL以检测恶意注入
- 检查进程依赖项
- 查找特定模块的加载地址用于Hook

**实现思路**：
1. 使用 `CreateToolhelp32Snapshot` 创建进程模块快照
2. 使用 `Module32First` 和 `Module32Next` 遍历模块
3. 对每个模块，填充 `ModuleInfo` 结构
4. 将所有模块信息收集到 `std::vector` 中返回

**注意事项**：
- 处理32位程序枚举64位进程模块的限制
- 处理权限不足的情况
- 确保正确释放快照句柄
- 考虑使用 `EnumProcessModules` 作为备选方案

### 4.5 ElevatePrivileges 提升进程权限

```cpp
static bool ElevatePrivileges(const std::wstring& privilege = L"SeDebugPrivilege");
```

**使用场景**：
- 需要访问其他进程内存时提升权限
- 执行系统级操作前获取必要权限
- 实现进程注入功能前的准备工作

**实现思路**：
1. 使用 `OpenProcessToken` 获取当前进程令牌
2. 使用 `LookupPrivilegeValue` 获取特权的LUID
3. 使用 `AdjustTokenPrivileges` 启用特权
4. 检查操作是否成功

**注意事项**：
- 需要管理员权限才能成功
- 处理UAC限制
- 确保正确释放令牌句柄
- 检查特权是否真正被启用，而不仅仅是函数返回成功

### 4.6 TerminateProcessById 终止进程

```cpp
static bool TerminateProcessById(DWORD pid);
```

**使用场景**：
- 终止检测到的恶意进程
- 强制关闭无响应的应用程序
- 系统清理时关闭不需要的进程

**实现思路**：
1. 使用 `OpenProcess` 获取进程句柄
2. 使用 `TerminateProcess` 终止进程
3. 检查操作是否成功

**注意事项**：
- 需要足够的权限才能终止其他进程
- 终止进程是强制性的，可能导致数据丢失
- 考虑先尝试更友好的方式关闭进程
- 确保正确释放进程句柄

### 4.7 GetCurrentProcessId 获取当前进程ID

```cpp
static DWORD GetCurrentProcessId();
```

**使用场景**：
- 记录日志时标识当前进程
- 防止自我注入或Hook
- 与其他进程通信时标识自身

**实现思路**：
1. 直接调用Windows API `::GetCurrentProcessId()`

**注意事项**：
- 这是一个简单的封装，主要为了API一致性
- 不需要错误处理，该API总是成功

### 4.8 GetParentProcessId 获取父进程ID

```cpp
static DWORD GetParentProcessId(DWORD pid);
```

**使用场景**：
- 分析进程创建链以检测可疑行为
- 验证进程的合法性
- 实现进程树显示功能

**实现思路**：
1. 使用 `CreateToolhelp32Snapshot` 创建进程快照
2. 使用 `Process32First` 和 `Process32Next` 遍历进程
3. 找到目标进程ID对应的进程项
4. 返回其父进程ID

**注意事项**：
- 处理目标进程可能已终止的情况
- 处理权限不足的情况
- 确保正确释放快照句柄

### 4.9 CreateProcessWithArgs 创建进程

```cpp
static bool CreateProcessWithArgs(const std::wstring& exePath, const std::wstring& args);
```

**使用场景**：
- 启动辅助工具或服务
- 执行命令行操作
- 实现自动化任务

**实现思路**：
1. 准备 `STARTUPINFO` 和 `PROCESS_INFORMATION` 结构
2. 组合可执行文件路径和命令行参数
3. 调用 `CreateProcess` API创建新进程
4. 检查操作是否成功
5. 关闭不需要的句柄

**注意事项**：
- 正确处理命令行参数，特别是包含空格的路径
- 考虑进程权限和UAC限制
- 确保正确释放进程和线程句柄
- 考虑是否需要等待进程完成

## 5. 性能优化建议

1. **缓存进程信息**：短时间内多次查询同一进程时，考虑缓存结果
2. **批量操作**：设计API支持批量获取多个进程信息，减少系统调用次数
3. **延迟加载**：只在需要时获取详细信息，而不是一次获取所有信息
4. **资源管理**：确保所有句柄都被正确关闭，避免句柄泄漏
5. **错误恢复**：实现备选方案，当主要API失败时尝试其他方法

## 6. 线程安全性

`ProcessUtils` 类的静态方法应该是线程安全的，因为它们不维护状态。但在实现时应注意以下几点：

1. **避免静态局部变量**：除非它们是常量或使用C++11的线程安全初始化
2. **参数和返回值**：确保通过值传递或const引用传递参数，避免修改调用者的数据
3. **系统资源**：确保在异常情况下也能正确释放系统资源（句柄等）
4. **API调用顺序**：某些Windows API的调用有顺序依赖，确保在多线程环境中也能正确工作

## 7. 错误处理策略

对于进程操作中可能出现的错误，建议采用以下策略：

1. **返回错误码或布尔值**：对于可能失败的操作，通过返回值指示成功或失败
2. **抛出异常**：对于严重错误或无法恢复的情况，抛出适当的异常
3. **日志记录**：记录详细的错误信息，包括Windows错误码和描述
4. **降级服务**：当无法获取完整信息时，尽可能返回部分信息而不是完全失败

## 8. 与Windows Hook技术的关联

`ProcessUtils` 类是实现Windows Hook技术的重要基础，主要体现在以下几个方面：

1. **目标进程识别**：通过进程名称或ID识别需要Hook的目标进程
2. **模块注入**：获取进程信息和权限，为DLL注入做准备
3. **内存地址定位**：通过模块信息找到需要Hook的函数地址
4. **权限管理**：提升权限以执行Hook操作
5. **进程监控**：监控系统进程的创建和终止，实现动态Hook

实现Hook技术时，`ProcessUtils` 类通常与 `StringUtils` 和 `Logger` 类配合使用，共同构成系统的核心功能三角。