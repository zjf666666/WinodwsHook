#pragma once

#include <Windows.h>

#include "IHook.h"

class InlineHook : public IHook
{
public:
    bool Init(const HookParam& params) override;

    bool Install() override;

    bool Uninstall() override;

    bool IsInstalled() const override;

    void* GetTrampolineAddress() const;

private:
    void* GetOriginalFunctionAddress() const;

    // 创建跳板函数
    /*
     * 偏移计算公式：偏移 = 目标函数地址 - （当前函数地址 + 指令长度）
     * CPU指令集执行方式：
     * 1. CPU读取并解码jmp指令，此时指令指针已经移动到下一个指令的首地址了
     * 2. CPU解析出要跳转的地址
     * 3. CPU将这个地址长度加到指令指针上得到要跳转的函数地址
     * 因此 jmp指令所在地址 + 指令长度 + 偏移长度 = 目标函数地址
     * jmp指令可以处理负地址，因此，我们不关心是向高字节挑还是低字节跳
     */
    bool CreateTrampolineFunc();

    bool Create32BitTrampolineFunc();

    bool Create64BitTrampolineFunc();

    void FreeTrampolineFunc();

    bool CreateCoverInst();

    bool Create32BitCoverInst();

    bool Create64BitCoverInst();

private:
    bool m_bIsInstalled;               // 是否已安装
    bool m_bIs64Bit;                   // 是否为64为程序
    void* m_pTargetAddress;            // 被HOOK的函数地址
    std::wstring m_wstrTargetModule;   // 目标模块路径
    std::string m_strTargetFuncName;   // 目标函数名称

    void* m_pHookFunction;             // Hook函数地址
    void* m_pTrampolineAddress;        // 跳板函数地址 一块单独分配的可读可写内存，保存了被替换的原始字节内容 + 跳回原函数后续内容的指令
    BYTE m_byteOriginal[16];           // 原始字节
    SIZE_T m_sizeParse;                // 补丁大小 x86 5字节， x64 12+字节
};

