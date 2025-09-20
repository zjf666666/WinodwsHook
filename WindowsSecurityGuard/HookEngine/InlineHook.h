#pragma once

#include <Windows.h>

#include "IHook.h"


// inline hook数据结构体
struct InlineHookContext : public BaseHookContext
{
    void* pHookFunction;             // Hook函数地址
    void* pTrampolineAddress;        // 跳板函数地址 一块单独分配的可读可写内存，保存了被替换的原始字节内容 + 跳回原函数后续内容的指令
    BYTE byteOriginal[16];           // 原始字节
    SIZE_T sizePatch;                // 补丁大小 x86 5字节， x64 12+字节
};

class InlineHook : public IHook
{
public:
    InlineHook(
        const std::wstring& targetModule,
        const std::string& targetFunction,
        void* hookFunction, // 我们实现的hook函数地址
        bool bIs64Bit
    );

    bool Install() override;

    bool Uninstall() override;

    bool IsInstalled() const override;

    bool IsEnabled() const override;

    bool Is64Bit() const override;

    void SetEnabled(bool enabled) override;

    void* GetTrampolineAddress() const;

    const std::wstring& GetTargetModule() const override;

    const std::string& GetTargetFunction() const override;

    const std::wstring& GetHookType() const override;

protected:
    void* GetOriginalFunctionAddress() const override;

private:
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

private:
    InlineHookContext m_inlineHookContext;
};

