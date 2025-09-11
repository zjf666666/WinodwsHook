# MemoryUtils 实现说明

## 1. 概述

MemoryUtils 是 Windows 安全防护系统中的核心工具类，提供了对进程内存的底层操作能力，是实现 Hook 技术和内存注入的基础支持组件。该工具类封装了 Windows 内存管理 API，提供了安全、高效、跨进程的内存操作接口，支持内存读写、保护属性修改、内存分配释放以及特征码扫描等功能。

## 2. 设计模式

### 静态工具类设计模式

**选择原因**：
1. **无状态操作**：内存操作通常是无状态的，不需要维护对象实例的状态信息
2. **全局访问**：系统各模块都可能需要内存操作功能，静态类可以方便地全局访问
3. **简化使用**：无需创建实例，直接通过类名调用方法，使用更加简洁
4. **资源效率**：避免创建多个实例导致的资源浪费

**实现方式**：
```cpp
class MemoryUtils {
public:
    // 静态方法，无需实例化
    static BOOL ReadMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead);
    static BOOL WriteMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);
    // 其他静态方法...

private:
    // 私有构造函数，防止实例化
    MemoryUtils() {}
    // 私有析构函数
    ~MemoryUtils() {}
    // 禁用拷贝构造和赋值操作
    MemoryUtils(const MemoryUtils&) = delete;
    MemoryUtils& operator=(const MemoryUtils&) = delete;
};
```

## 3. 数据结构定义

### 3.1 内存区域信息结构体

```cpp
// 注意：此结构体在当前代码中未定义
struct MemoryRegionInfo {
    PVOID BaseAddress;       // 内存区域基址
    SIZE_T RegionSize;      // 内存区域大小
    DWORD Protect;          // 内存保护属性
    DWORD State;            // 内存状态（MEM_COMMIT, MEM_RESERVE, MEM_FREE）
    DWORD Type;             // 内存类型（MEM_PRIVATE, MEM_MAPPED, MEM_IMAGE）
};
```

### 3.2 内存模式搜索结构体

```cpp
// 注意：此结构体在当前代码中未定义
struct MemoryPattern {
    std::vector<BYTE> Pattern;       // 要搜索的字节模式
    std::string Mask;               // 模式掩码，'x'表示必须匹配，'?'表示可以忽略
    
    MemoryPattern(const std::vector<BYTE>& pattern, const std::string& mask)
        : Pattern(pattern), Mask(mask) {}
        
    // 支持从十六进制字符串创建模式
    static MemoryPattern FromHexString(const std::string& hexString);
};
```

## 4. 函数实现说明

### 4.1 ReadMemory 函数

**函数声明**：
```cpp
/* 
 * @brief 从指定进程的内存地址读取数据
 * @param [IN] hProcess 目标进程的句柄，需要具有PROCESS_VM_READ权限
 *        [IN] lpBaseAddress 要读取的内存起始地址
 *        [OUT] lpBuffer 接收读取数据的缓冲区
 *        [IN] nSize 要读取的字节数
 *        [OUT] lpNumberOfBytesRead 实际读取的字节数
 * @return bool 成功返回true，失败返回false
 */
static bool ReadMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead);
```

**实现思路**：
1. 封装 Windows API `ReadProcessMemory`，提供更安全的内存读取操作
2. 添加参数验证，确保传入的参数有效
3. 处理读取失败的情况，提供详细的错误信息
4. 支持读取当前进程和远程进程的内存

**注意事项**：
- 读取远程进程内存时，需要确保进程句柄具有 `PROCESS_VM_READ` 权限
- 读取的内存地址必须是有效的，否则会导致操作失败
- 对于大块内存的读取，应考虑分块读取以提高成功率

**使用示例**：
```cpp
// 读取当前进程内存
BYTE buffer[100];
SIZE_T bytesRead;
MemoryUtils::ReadMemory(GetCurrentProcess(), (LPCVOID)0x12345678, buffer, sizeof(buffer), &bytesRead);

// 读取远程进程内存
HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, dwProcessId);
if (hProcess) {
    MemoryUtils::ReadMemory(hProcess, (LPCVOID)0x12345678, buffer, sizeof(buffer), &bytesRead);
    CloseHandle(hProcess);
}
```

### 4.2 WriteMemory 函数

**函数声明**：
```cpp
/* 
 * @brief 向指定进程的内存地址写入数据
 * @param [IN] hProcess 目标进程的句柄，需要具有PROCESS_VM_WRITE和PROCESS_VM_OPERATION权限
 *        [IN] lpBaseAddress 要写入的内存起始地址
 *        [IN] lpBuffer 包含要写入数据的缓冲区
 *        [IN] nSize 要写入的字节数
 *        [OUT] lpNumberOfBytesWritten 实际写入的字节数
 *        [IN] flAllocationType 内存分配类型
 *        [IN] flProtect 内存保护属性
 *        [IN] bIsRealloc 指定地址分配失败是否使用系统默认分配重新分配一块
 * @return bool 成功返回true，失败返回false
 */
static bool WriteMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T nSize,
    SIZE_T* lpNumberOfBytesWritten,
    DWORD flAllocationType = MEM_COMMIT | MEM_RESERVE,
    DWORD flProtect = PAGE_READWRITE,
    bool bIsRealloc = false
);
```

**实现思路**：
1. 封装 Windows API `WriteProcessMemory`，提供更安全的内存写入操作
2. 添加参数验证，确保传入的参数有效
3. 处理写入失败的情况，提供详细的错误信息
4. 支持写入当前进程和远程进程的内存

**注意事项**：
- 写入远程进程内存时，需要确保进程句柄具有 `PROCESS_VM_WRITE` 和 `PROCESS_VM_OPERATION` 权限
- 写入的内存地址必须是可写的，可能需要先修改内存保护属性
- 写入代码段时，需要考虑指令缓存的刷新问题

**使用示例**：
```cpp
// 写入当前进程内存
BYTE data[5] = {0x90, 0x90, 0x90, 0x90, 0x90}; // NOP指令
SIZE_T bytesWritten;
MemoryUtils::WriteMemory(GetCurrentProcess(), (LPVOID)0x12345678, data, sizeof(data), &bytesWritten);

// 写入远程进程内存
HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, dwProcessId);
if (hProcess) {
    MemoryUtils::WriteMemory(hProcess, (LPVOID)0x12345678, data, sizeof(data), &bytesWritten);
    CloseHandle(hProcess);
}
```

### 4.3 ModifyMemoryProtection 函数

**函数声明**：
```cpp
// 注意：此函数在当前代码中未实现
static bool ModifyMemoryProtection(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
```

**实现思路**：
1. 封装 Windows API `VirtualProtectEx`，提供更安全的内存保护属性修改操作
2. 添加参数验证，确保传入的参数有效
3. 处理修改失败的情况，提供详细的错误信息
4. 支持修改当前进程和远程进程的内存保护属性

**注意事项**：
- 修改远程进程内存保护属性时，需要确保进程句柄具有 `PROCESS_VM_OPERATION` 权限
- 修改完内存操作后，应该恢复原来的保护属性，避免安全问题
- 不同的内存区域可能有不同的最小粒度，需要按照页面边界对齐

**使用示例**：
```cpp
// 修改当前进程内存保护属性
DWORD oldProtect;
MemoryUtils::ModifyMemoryProtection(GetCurrentProcess(), (LPVOID)0x12345678, 100, PAGE_EXECUTE_READWRITE, &oldProtect);
// 执行内存操作...
MemoryUtils::ModifyMemoryProtection(GetCurrentProcess(), (LPVOID)0x12345678, 100, oldProtect, nullptr);

// 修改远程进程内存保护属性
HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION, FALSE, dwProcessId);
if (hProcess) {
    MemoryUtils::ModifyMemoryProtection(hProcess, (LPVOID)0x12345678, 100, PAGE_EXECUTE_READWRITE, &oldProtect);
    // 执行内存操作...
    MemoryUtils::ModifyMemoryProtection(hProcess, (LPVOID)0x12345678, 100, oldProtect, nullptr);
    CloseHandle(hProcess);
}
```

### 4.4 AllocateMemory 函数

**函数声明**：
```cpp
// 注意：此函数在当前代码中未实现
static LPVOID AllocateMemory(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
```

**实现思路**：
1. 封装 Windows API `VirtualAllocEx`，提供更安全的内存分配操作
2. 添加参数验证，确保传入的参数有效
3. 处理分配失败的情况，提供详细的错误信息
4. 支持在当前进程和远程进程中分配内存

**注意事项**：
- 分配远程进程内存时，需要确保进程句柄具有 `PROCESS_VM_OPERATION` 权限
- 分配的内存应该在使用完毕后释放，避免内存泄漏
- 考虑内存分配的对齐要求，特别是在执行代码注入时

**使用示例**：
```cpp
// 在当前进程分配内存
LPVOID memAddr = MemoryUtils::AllocateMemory(GetCurrentProcess(), nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
if (memAddr) {
    // 使用内存...
    MemoryUtils::FreeMemory(GetCurrentProcess(), memAddr, 0, MEM_RELEASE);
}

// 在远程进程分配内存
HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION, FALSE, dwProcessId);
if (hProcess) {
    LPVOID remoteAddr = MemoryUtils::AllocateMemory(hProcess, nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remoteAddr) {
        // 使用远程内存...
        MemoryUtils::FreeMemory(hProcess, remoteAddr, 0, MEM_RELEASE);
    }
    CloseHandle(hProcess);
}
```

### 4.5 FreeMemory 函数

**函数声明**：
```cpp
// 注意：此函数在当前代码中未实现
static bool FreeMemory(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
```

**实现思路**：
1. 封装 Windows API `VirtualFreeEx`，提供更安全的内存释放操作
2. 添加参数验证，确保传入的参数有效
3. 处理释放失败的情况，提供详细的错误信息
4. 支持释放当前进程和远程进程的内存

**注意事项**：
- 释放远程进程内存时，需要确保进程句柄具有 `PROCESS_VM_OPERATION` 权限
- 释放内存时，需要使用正确的释放类型（MEM_DECOMMIT 或 MEM_RELEASE）
- 释放 MEM_RELEASE 时，dwSize 参数必须为 0

**使用示例**：
```cpp
// 释放当前进程内存
MemoryUtils::FreeMemory(GetCurrentProcess(), memAddr, 0, MEM_RELEASE);

// 释放远程进程内存
HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION, FALSE, dwProcessId);
if (hProcess) {
    MemoryUtils::FreeMemory(hProcess, remoteAddr, 0, MEM_RELEASE);
    CloseHandle(hProcess);
}
```

### 4.6 FindPattern 函数

**函数声明**：
```cpp
// 注意：此函数在当前代码中未实现
static PVOID FindPattern(HANDLE hProcess, PVOID startAddress, SIZE_T searchSize, const MemoryPattern& pattern);
```

**实现思路**：
1. 在指定的内存范围内搜索匹配的字节模式
2. 支持使用掩码进行模糊匹配，'x'表示必须匹配，'?'表示可以忽略
3. 分块读取内存，避免一次读取过大的内存块
4. 返回找到的第一个匹配位置的地址

**注意事项**：
- 搜索大范围内存时，性能可能会受到影响，应考虑优化搜索算法
- 搜索远程进程内存时，需要确保进程句柄具有 `PROCESS_VM_READ` 权限
- 模式和掩码的长度必须匹配，否则会导致搜索结果不准确

**使用示例**：
```cpp
// 在当前进程中搜索特征码
std::vector<BYTE> pattern = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x20};
std::string mask = "xxxxxx";
MemoryPattern searchPattern(pattern, mask);

PVOID foundAddress = MemoryUtils::FindPattern(
    GetCurrentProcess(),
    (PVOID)0x10000000,  // 起始地址
    0x1000000,          // 搜索范围
    searchPattern
);

// 在远程进程中搜索特征码
HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, dwProcessId);
if (hProcess) {
    PVOID foundAddress = MemoryUtils::FindPattern(
        hProcess,
        (PVOID)0x10000000,  // 起始地址
        0x1000000,          // 搜索范围
        searchPattern
    );
    CloseHandle(hProcess);
}
```

### 4.7 SafeCloseHandle 函数

**函数声明**：
```cpp
static void SafeCloseHandle(HANDLE hHandle, HANDLE hResetValue = NULL);
```

**实现思路**：
1. 安全关闭句柄，避免句柄泄漏
2. 检查句柄是否有效，只关闭有效的句柄
3. 关闭后将句柄重置为指定值，默认为NULL

**注意事项**：
- 只有非NULL且非INVALID_HANDLE_VALUE的句柄才会被关闭
- 关闭后将句柄重置为指定值，避免悬挂指针
- 适用于各种Windows句柄类型

**使用示例**：
```cpp
HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
if (hProcess) {
    // 使用进程句柄...
    MemoryUtils::SafeCloseHandle(hProcess);
}
```

### 4.8 CreateRemoteThread 函数

**函数声明**：
```cpp
// 注意：此函数在当前代码中未实现
static HANDLE CreateRemoteThread(HANDLE hProcess, LPVOID lpStartAddress, LPVOID lpParameter);
```

**实现思路**：
1. 封装 Windows API `CreateRemoteThread`，提供更简洁的远程线程创建接口
2. 添加参数验证，确保传入的参数有效
3. 处理创建失败的情况，提供详细的错误信息
4. 支持自定义线程参数和安全属性

**注意事项**：
- 创建远程线程时，需要确保进程句柄具有 `PROCESS_CREATE_THREAD`、`PROCESS_VM_OPERATION`、`PROCESS_VM_WRITE` 和 `PROCESS_VM_READ` 权限
- 远程线程的起始地址必须在目标进程的地址空间中是有效的
- 创建的线程句柄在不需要时应该关闭，避免句柄泄漏

**使用示例**：
```cpp
// 在远程进程中创建线程
HANDLE hProcess = OpenProcess(
    PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
    FALSE,
    dwProcessId
);

if (hProcess) {
    // 假设已经在远程进程中分配了内存并写入了代码
    LPVOID remoteFunction = /* 远程进程中的函数地址 */;
    LPVOID remoteParam = /* 远程进程中的参数地址 */;
    
    HANDLE hThread = MemoryUtils::CreateRemoteThread(hProcess, remoteFunction, remoteParam);
    if (hThread) {
        // 等待线程完成
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
    
    CloseHandle(hProcess);
}
```

## 5. 性能优化建议

### 5.1 内存操作批处理

对于需要频繁进行的小块内存操作，可以考虑合并为批量操作，减少系统调用的开销：

```cpp
// 优化前：多次小块读取
for (int i = 0; i < 100; i++) {
    BYTE value;
    MemoryUtils::ReadMemory(hProcess, (LPCVOID)(baseAddr + i), &value, 1, nullptr);
    // 处理 value...
}

// 优化后：一次性读取
BYTE buffer[100];
MemoryUtils::ReadMemory(hProcess, (LPCVOID)baseAddr, buffer, 100, nullptr);
for (int i = 0; i < 100; i++) {
    // 处理 buffer[i]...
}
```

### 5.2 内存缓存策略

对于频繁访问的内存区域，可以实现缓存机制，减少重复读取的开销：

```cpp
class MemoryCache {
private:
    std::unordered_map<DWORD_PTR, std::vector<BYTE>> cache;
    HANDLE hProcess;
    
public:
    MemoryCache(HANDLE process) : hProcess(process) {}
    
    BYTE* GetMemory(DWORD_PTR address, SIZE_T size) {
        // 检查缓存
        auto it = cache.find(address);
        if (it != cache.end() && it->second.size() >= size) {
            return it->second.data();
        }
        
        // 缓存未命中，读取内存
        std::vector<BYTE> buffer(size);
        if (MemoryUtils::ReadMemory(hProcess, (LPCVOID)address, buffer.data(), size, nullptr)) {
            cache[address] = buffer;
            return buffer.data();
        }
        
        return nullptr;
    }
    
    void InvalidateCache() {
        cache.clear();
    }
};
```

### 5.3 内存搜索算法优化

对于大范围的内存模式搜索，可以使用更高效的算法，如 Boyer-Moore 或 KMP 算法：

```cpp
// 使用 Boyer-Moore 算法优化模式搜索
static PVOID FindPatternBM(HANDLE hProcess, PVOID startAddress, SIZE_T searchSize, const MemoryPattern& pattern) {
    // 实现 Boyer-Moore 搜索算法...
}
```

### 5.4 并行处理

对于大范围的内存操作，可以考虑使用多线程并行处理：

```cpp
static std::vector<PVOID> FindPatternParallel(HANDLE hProcess, PVOID startAddress, SIZE_T searchSize, const MemoryPattern& pattern, int threadCount = 4) {
    std::vector<PVOID> results;
    std::vector<std::thread> threads;
    std::mutex resultMutex;
    
    SIZE_T chunkSize = searchSize / threadCount;
    
    for (int i = 0; i < threadCount; i++) {
        PVOID chunkStart = (BYTE*)startAddress + (i * chunkSize);
        SIZE_T actualChunkSize = (i == threadCount - 1) ? (searchSize - i * chunkSize) : chunkSize;
        
        threads.push_back(std::thread([=, &results, &resultMutex]() {
            PVOID found = FindPattern(hProcess, chunkStart, actualChunkSize, pattern);
            if (found) {
                std::lock_guard<std::mutex> lock(resultMutex);
                results.push_back(found);
            }
        }));
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    return results;
}
```

## 6. 线程安全性

### 6.1 线程安全考虑

MemoryUtils 作为静态工具类，本身不维护状态，因此大多数方法都是线程安全的。但在使用时仍需注意以下几点：

1. **共享句柄**：多线程共享同一个进程句柄时，需要确保句柄不会被提前关闭
2. **并发写入**：多线程同时写入同一内存区域可能导致数据竞争
3. **内存保护修改**：多线程修改同一内存区域的保护属性可能导致冲突

### 6.2 线程安全实现建议

```cpp
// 使用互斥锁保护关键操作
static std::mutex memoryMutex;

static BOOL WriteMemorySafe(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten) {
    std::lock_guard<std::mutex> lock(memoryMutex);
    return WriteMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}

static BOOL ModifyMemoryProtectionSafe(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect) {
    std::lock_guard<std::mutex> lock(memoryMutex);
    return ModifyMemoryProtection(hProcess, lpAddress, dwSize, flNewProtect, lpflOldProtect);
}
```

## 7. 兼容性考虑

### 7.1 不同 Windows 版本的兼容性

MemoryUtils 需要考虑在不同 Windows 版本上的兼容性问题：

1. **API 可用性**：某些内存操作 API 可能在不同 Windows 版本中有差异
2. **安全机制**：较新的 Windows 版本可能有更严格的内存保护机制
3. **地址空间布局**：64位和32位系统的地址空间布局差异

### 7.2 兼容性实现建议

```cpp
// 检测系统版本并使用适当的API
static BOOL IsWindows8OrLater() {
    OSVERSIONINFOEX osvi = {sizeof(OSVERSIONINFOEX)};
    DWORDLONG dwlConditionMask = 0;
    
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    
    osvi.dwMajorVersion = 6;
    osvi.dwMinorVersion = 2; // Windows 8
    
    return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask);
}

static LPVOID AllocateMemoryCompat(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
    if (IsWindows8OrLater()) {
        // 使用 Windows 8 及更高版本的增强API或参数
        return AllocateMemory(hProcess, lpAddress, dwSize, flAllocationType | MEM_TOP_DOWN, flProtect);
    } else {
        // 使用基本API
        return AllocateMemory(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
    }
}
```

## 8. 安全性考虑

### 8.1 权限检查

在执行内存操作前，应该检查当前进程是否具有足够的权限：

```cpp
static BOOL CheckProcessAccess(HANDLE hProcess, DWORD desiredAccess) {
    if (hProcess == GetCurrentProcess()) {
        return TRUE; // 当前进程总是有足够权限
    }
    
    DWORD flags;
    if (GetHandleInformation(hProcess, &flags)) {
        // 检查句柄是否有效且具有所需权限
        // 这只是一个简单检查，实际上需要更复杂的验证
        return TRUE;
    }
    
    return FALSE;
}

static BOOL WriteMemorySafe(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten) {
    if (!CheckProcessAccess(hProcess, PROCESS_VM_WRITE | PROCESS_VM_OPERATION)) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
    
    return WriteMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}
```

### 8.2 地址验证

在操作内存前，应该验证地址的有效性，避免访问无效内存：

```cpp
static BOOL IsValidMemoryRange(HANDLE hProcess, LPCVOID lpAddress, SIZE_T dwSize) {
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(hProcess, lpAddress, &mbi, sizeof(mbi)) == 0) {
        return FALSE;
    }
    
    // 检查内存是否已提交
    if (mbi.State != MEM_COMMIT) {
        return FALSE;
    }
    
    // 检查内存范围是否在同一个内存区域内
    if ((BYTE*)lpAddress + dwSize > (BYTE*)mbi.BaseAddress + mbi.RegionSize) {
        return FALSE;
    }
    
    return TRUE;
}

static BOOL ReadMemorySafe(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead) {
    if (!IsValidMemoryRange(hProcess, lpBaseAddress, nSize)) {
        SetLastError(ERROR_INVALID_ADDRESS);
        return FALSE;
    }
    
    return ReadMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
}
```

## 9. 调试与诊断

### 9.1 错误日志

为了便于调试和诊断问题，MemoryUtils 应该提供详细的错误日志：

```cpp
static void LogMemoryError(const std::wstring& operation, DWORD errorCode) {
    std::wstring errorMessage = StringUtils::FormatString(
        L"内存操作 '%s' 失败，错误码: %d，错误信息: %s",
        operation.c_str(),
        errorCode,
        StringUtils::GetErrorMessage(errorCode).c_str()
    );
    
    Logger::GetInstance().Error(errorMessage);
}

static BOOL WriteMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten) {
    BOOL result = WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
    if (!result) {
        DWORD errorCode = GetLastError();
        LogMemoryError(L"WriteMemory", errorCode);
    }
    return result;
}
```

### 9.2 内存操作跟踪

在调试模式下，可以添加内存操作跟踪功能，记录所有内存读写操作：

```cpp
#ifdef _DEBUG
static void TraceMemoryOperation(const std::wstring& operation, HANDLE hProcess, LPCVOID lpAddress, SIZE_T nSize) {
    DWORD processId = GetProcessId(hProcess);
    std::wstring traceMessage = StringUtils::FormatString(
        L"内存操作: %s, 进程ID: %d, 地址: 0x%p, 大小: %zu 字节",
        operation.c_str(),
        processId,
        lpAddress,
        nSize
    );
    
    Logger::GetInstance().Debug(traceMessage);
}
#else
#define TraceMemoryOperation(operation, hProcess, lpAddress, nSize)
#endif

static BOOL ReadMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead) {
    TraceMemoryOperation(L"ReadMemory", hProcess, lpBaseAddress, nSize);
    return ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
}
```

## 10. 总结

MemoryUtils 工具类是 Windows 安全防护系统中的核心组件，为 Hook 技术和内存注入提供了基础支持。通过封装 Windows 内存管理 API，提供了安全、高效、跨进程的内存操作接口，支持内存读写、保护属性修改、内存分配释放以及特征码扫描等功能。

在实现 MemoryUtils 时，需要特别注意以下几点：

1. **安全性**：验证内存操作的权限和地址有效性，防止非法访问
2. **性能**：优化内存操作算法，减少系统调用开销，提高大范围内存搜索的效率
3. **兼容性**：考虑不同 Windows 版本和系统架构的差异，确保在各种环境下正常工作
4. **线程安全**：保证在多线程环境下的正确性，避免数据竞争和资源冲突
5. **错误处理**：提供详细的错误信息和日志，便于问题诊断和调试

通过合理设计和实现 MemoryUtils 工具类，可以为 Windows Hook 技术提供强大的底层支持，提高系统的可靠性、可维护性和安全性。