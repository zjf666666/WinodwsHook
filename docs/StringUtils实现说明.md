# StringUtils 实现说明文档

## 1. 概述

`StringUtils` 是 Windows 安全防护系统中的核心工具类，提供字符串处理的基础功能。该类设计为纯静态工具类，不需要实例化，所有方法均为静态方法，便于在系统各处直接调用。

## 2. 函数实现说明

### 2.1 WideToMultiByte 宽字符转多字节

```cpp
static std::string WideToMultiByte(const std::wstring& wide);
```

**实现思路**：
1. 使用 `WideCharToMultiByte` Windows API 函数进行字符集转换
2. 首先调用该函数获取所需缓冲区大小
3. 分配足够的缓冲区空间
4. 再次调用函数执行实际转换
5. 将结果包装为 `std::string` 返回

**注意事项**：
- 需要处理转换失败的情况，返回空字符串或抛出异常
- 默认使用当前系统代码页（CP_ACP），可考虑支持指定代码页的重载版本
- 需要正确处理缓冲区大小计算，避免内存泄漏或缓冲区溢出

### 2.2 MultiByteToWide 多字节转宽字符

```cpp
static std::wstring MultiByteToWide(const std::string& mb);
```

**实现思路**：
1. 使用 `MultiByteToWideChar` Windows API 函数进行字符集转换
2. 首先调用该函数获取所需缓冲区大小
3. 分配足够的缓冲区空间
4. 再次调用函数执行实际转换
5. 将结果包装为 `std::wstring` 返回

**注意事项**：
- 需要处理转换失败的情况
- 默认使用当前系统代码页，可考虑支持指定代码页的重载版本
- 正确处理空字符串输入的情况

### 2.3 FormatString 格式化字符串

```cpp
static std::wstring FormatString(std::wstring format, ...);
```

**实现思路**：
1. 使用 C++ 可变参数模板和 `vswprintf` 函数实现格式化
2. 处理可变参数列表
3. 动态分配足够大小的缓冲区
4. 执行格式化操作
5. 将结果转换为 `std::wstring` 返回

**注意事项**：
- 需要正确处理可变参数列表，避免内存泄漏
- 考虑缓冲区大小估算，避免溢出
- 处理格式化失败的情况
- 可能需要多次尝试以确定足够的缓冲区大小

### 2.4 SplitString 字符串分割

```cpp
static std::vector<std::wstring> SplitString(const std::wstring& str, wchar_t delimiter);
```

**实现思路**：
1. 创建一个 `std::vector<std::wstring>` 用于存储分割结果
2. 使用 `std::wstringstream` 和 `std::getline` 函数进行分割
3. 或者手动遍历字符串，查找分隔符位置
4. 提取子字符串并添加到结果向量中

**注意事项**：
- 处理空字符串输入的情况
- 处理连续分隔符的情况（是否生成空字符串元素）
- 考虑字符串首尾有分隔符的边界情况
- 可以考虑添加一个重载版本，支持使用字符串作为分隔符

### 2.5 NormalizePath 路径规范化

```cpp
static std::wstring NormalizePath(const std::wstring& path);
```

**实现思路**：
1. 将所有反斜杠 `\` 转换为正斜杠 `/`（或相反，取决于系统标准）
2. 处理连续的斜杠，将其合并为单个斜杠
3. 处理相对路径符号，如 `./` 和 `../`
4. 移除路径末尾的斜杠（可选，取决于需求）
5. 处理特殊路径，如 UNC 路径

**注意事项**：
- 需要正确处理 Windows 特有的路径格式，如驱动器号（C:）
- 处理 UNC 路径（\\server\share）的特殊情况
- 考虑使用 Windows API 如 `PathCanonicalize` 辅助实现

### 2.6 ReplaceString 字符串替换

```cpp
static std::wstring ReplaceString(const std::wstring& str, const std::wstring& from, const std::wstring& to);
```

**实现思路**：
1. 检查原始字符串和要替换的子字符串是否为空
2. 创建结果字符串的副本
3. 使用循环查找并替换所有匹配的子字符串
4. 可以使用 `std::wstring::find` 和 `std::wstring::replace` 方法
5. 或者手动实现查找和替换逻辑

**注意事项**：
- 处理 `from` 为空字符串的特殊情况
- 考虑替换后的字符串可能包含原 `from` 字符串的情况，避免无限循环
- 优化性能，避免频繁的内存分配和复制

### 2.7 ToLower 字符串转小写

```cpp
static std::wstring ToLower(const std::wstring& str);
```

**实现思路**：
1. 创建原始字符串的副本
2. 使用 `std::transform` 和 `std::towlower` 函数将所有字符转换为小写
3. 或者手动遍历字符串，将每个字符转换为小写

**注意事项**：
- 考虑国际化和本地化问题，某些语言的大小写转换规则可能不同
- 可以使用 Windows API 如 `CharLower` 处理特定语言环境的转换

### 2.8 ToUpper 字符串转大写

```cpp
static std::wstring ToUpper(const std::wstring& str);
```

**实现思路**：
1. 创建原始字符串的副本
2. 使用 `std::transform` 和 `std::towupper` 函数将所有字符转换为大写
3. 或者手动遍历字符串，将每个字符转换为大写

**注意事项**：
- 与 ToLower 类似，需要考虑国际化和本地化问题
- 可以使用 Windows API 如 `CharUpper` 处理特定语言环境的转换

## 3. 性能优化建议

1. **避免不必要的字符串复制**：在可能的情况下，使用引用传递和返回
2. **预分配内存**：对于可能增长的字符串操作，预先分配足够的内存空间
3. **使用适当的 STL 算法**：如 `std::transform`、`std::find` 等，它们通常比手动循环更高效
4. **考虑使用 `std::string_view`**：对于不需要修改的字符串参数，可以考虑使用 C++17 的 `std::string_view`
5. **缓存计算结果**：对于频繁调用的转换函数，考虑实现缓存机制
6. **使用 Windows API**：在适当的情况下，直接使用 Windows API 可能比标准库函数更高效

## 4. 线程安全性

由于 `StringUtils` 是纯静态工具类，不维护任何状态，所有方法都是无状态的，因此天然是线程安全的。但在实现时仍需注意：

1. 避免使用静态局部变量，除非它们是常量或线程安全的
2. 确保临时缓冲区不会在线程间共享
3. 对于可能的共享资源（如缓存），需要实现适当的同步机制

## 5. 错误处理策略

对于字符串处理中可能出现的错误，建议采用以下策略：

1. **返回空字符串或默认值**：对于非关键操作，可以在失败时返回空字符串或其他默认值
2. **使用错误码**：对于可能失败的操作，可以返回错误码或使用 `std::pair<std::wstring, bool>` 表示结果和成功状态
3. **抛出异常**：对于严重错误或无法恢复的情况，可以抛出适当的异常
4. **日志记录**：记录错误信息，但不中断程序执行

## 6. 测试建议

为确保 `StringUtils` 类的可靠性，建议进行以下测试：

1. **单元测试**：测试每个函数的基本功能和边界情况
2. **性能测试**：测试大字符串和频繁调用场景下的性能
3. **国际化测试**：测试不同语言和字符集的处理能力
4. **内存泄漏测试**：确保没有内存泄漏，特别是在异常情况下
5. **线程安全测试**：在多线程环境中测试函数行为

### 2.9 WideToMultiByteC C风格字符串转换

```cpp
int WideToMultiByteC(const wchar_t* wide, int wideLen, char* buffer, int bufferSize);
```

**实现思路**：
1. 提供C风格接口，用于跨模块调用场景
2. 使用 `WideCharToMultiByte` Windows API 函数进行字符集转换
3. 直接操作传入的缓冲区
4. 返回转换后的字符数量或错误码

**注意事项**：
- 需要确保传入的缓冲区足够大
- 适用于需要避免C++字符串兼容性问题的跨模块调用场景
- 调用者负责管理缓冲区内存

## 7. 注释规范

按照项目文档中定义的注释规范，所有 `StringUtils` 类的函数都应该有详细的注释，包括：

```cpp
/**
 * @brief 简要描述函数功能
 * @param [IN/OUT] 参数名 参数详细说明
 * @return 返回值说明
 * @exception 可能抛出的异常说明（如适用）
 * @note 其他需要说明的事项（如适用）
 */
```

例如，对于 `ReplaceString` 函数：

```cpp
/**
 * @brief 在字符串中查找并替换所有指定的子字符串
 * @param [IN] str 原始字符串，需要进行替换操作的源字符串
 * @param [IN] from 需要被替换的子字符串
 * @param [IN] to 替换成的新子字符串
 * @return std::wstring 替换操作完成后的新字符串
 */
static std::wstring ReplaceString(const std::wstring& str, const std::wstring& from, const std::wstring& to);
```