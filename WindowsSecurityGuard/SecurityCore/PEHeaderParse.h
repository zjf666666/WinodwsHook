#pragma once

#include <string>
#include <vector>

#include <Windows.h>

enum class Architecture {
    X86,
    X64,
    Unknown
};

/* 
 * PE头解析工具类
 * 我希望这个类可以传出每个类的具体内容，如NT头、DOS头、导入表、导出表
 * 而这些内容的获取依赖于前面的解析结果的，因此每个函数之间存在依赖关系
 * 因此这是有状态的，将这个类设计为实例工具类，保存PE头结构体作为私有成员
 * 构造时传入PE文件首部地址，会自动在内部保存所有状态
 */
class PEHeaderParse {
public:
    PEHeaderParse(const UINT8* base);

private:
    // 辅助函数
    bool GetNTHeaders(void* moduleBase);
    bool GetSectionHeader(void* moduleBase, const char* sectionName);

private:
    const UINT8* prtAddress;
    Architecture arch; // 架构信息
    IMAGE_DOS_HEADER dosHeader; // DOS头

    IMAGE_NT_HEADERS32 ntHeader32; // 32位NT头
    IMAGE_NT_HEADERS64 ntHeader64; // 64位NT头

    IMAGE_FILE_HEADER fileHeader; // 文件头

    IMAGE_OPTIONAL_HEADER32 opHeader32; // 32位可选头
    IMAGE_OPTIONAL_HEADER64 opHeader64; // 64位可选头

    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

