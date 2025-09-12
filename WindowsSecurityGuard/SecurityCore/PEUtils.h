#pragma once

#include <string>
#include <vector>

#include <Windows.h>

/* PE头解析工具类 */
class PEUtils {
public:
    // 架构相关
    static bool Is64BitPE(const std::wstring& filePath);
    static bool Is64BitPE(void* moduleBase);
    static bool IsSameMachineType(void* moduleA, void* moduleB);

    // 节区分析
    static bool HasExecutableSection(void* moduleBase);
    static bool IsAddressInExecutableSection(void* moduleBase, void* address);

    // 导入表/导出表分析
    static bool GetImportedFunctions(void* moduleBase, std::vector<std::string>& functions);
    static bool GetExportedFunctions(void* moduleBase, std::vector<std::string>& functions);

    // 其他PE信息
    static DWORD GetEntryPoint(void* moduleBase);
    static bool IsManaged(void* moduleBase);  // 检测.NET程序
    static bool IsSigned(void* moduleBase);   // 检测数字签名

private:
    // 辅助函数
    static PIMAGE_NT_HEADERS GetNTHeaders(void* moduleBase);
    static PIMAGE_SECTION_HEADER GetSectionHeader(void* moduleBase, const char* sectionName);

private:
    /*
     * 以下代码为规范性代码，工具类应避免显式生成对象，使用::的形式进行调用
     * 删除拷贝构造函数及拷贝赋值函数避免友元函数或成员函数进行拷贝操作
     */
    PEUtils() = delete;
    ~PEUtils() = delete;
    PEUtils(const PEUtils&) = delete;
    PEUtils& operator=(const PEUtils&) = delete;
};

