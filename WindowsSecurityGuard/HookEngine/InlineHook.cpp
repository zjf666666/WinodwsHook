#include "pch.h"
#include "InlineHook.h"

#include "../SecurityCore/Logger.h"
#include "../SecurityCore/ProcessUtils.h"
#include "../SecurityCore/VirtualMemoryWrapper.h"
#include "../SecurityCore//HandleWrapper.h"
#include "InstructionParser.h"
#include "InstructionRelocator.h"

#define LEN_JUMP_BYTE_32   5  // 32位进程jmp指令长度

InlineHook::InlineHook(
    const std::wstring& targetModule,
    const std::string& targetFunction,
    void* hookFunction,
    bool bIs64Bit
)
{
    m_inlineHookContext.bIs64Bit = bIs64Bit;
    m_inlineHookContext.bIsEnabled = true;
    m_inlineHookContext.bIsInstalled = false;
    m_inlineHookContext.pHookFunction = hookFunction;
    m_inlineHookContext.pTargetAddress = nullptr;
    m_inlineHookContext.pTrampolineAddress = nullptr;
    m_inlineHookContext.strTargetFuncName = targetFunction;
    m_inlineHookContext.wstrTargetModule = targetModule;
}

bool InlineHook::Install()
{
    // 检测参数是否合法
    if (m_inlineHookContext.bIsInstalled)
    {
        // 如果已安装 返回true，记录info日志
        Logger::GetInstance().Info(L"Inline hook has already installed");
        return true;
    }

    if (m_inlineHookContext.wstrTargetModule.empty() ||
        m_inlineHookContext.strTargetFuncName.empty() ||
        nullptr == m_inlineHookContext.pHookFunction)
    {
        // 模块路径、函数名称、hook函数地址三个数据均不能为空
        Logger::GetInstance().Error(L"Inline hook param error!");
        return false;
    }

    // 获取原函数地址并保存
    m_inlineHookContext.pTargetAddress = GetOriginalFunctionAddress();

    // 填充跳板函数
    CreateTrampolineFunc();

    // 修改内存页状态为可读可写
    VirtualProtectWrapper virtualWrapper(m_inlineHookContext.pTargetAddress, m_inlineHookContext.sizePatch, PAGE_EXECUTE_READWRITE);
    if (!virtualWrapper.IsValid())
    {
        Logger::GetInstance().Error(L"Inline hook param error!");
        FreeTrampolineFunc();
        return false;
    }

    // 写入jmp指令
    unsigned char* pTargetPtr = (unsigned char*)m_inlineHookContext.pTargetAddress;
    *pTargetPtr = 0xE9;

    // 计算偏移
    DWORD dwOffset = (uintptr_t)m_inlineHookContext.pHookFunction - ((uintptr_t)pTargetPtr + LEN_JUMP_BYTE_32);

    // 将偏移写入内存
    memcpy(pTargetPtr + 1, &dwOffset, LEN_JUMP_BYTE_32 - 1);

    m_inlineHookContext.bIsInstalled = true;

    // virtualWrapper 析构时自动还原内存保护属性
    return true;
}

bool InlineHook::Uninstall()
{
    FreeTrampolineFunc();
    return false;
}

bool InlineHook::IsInstalled() const
{
    return m_inlineHookContext.bIsInstalled;
}

bool InlineHook::IsEnabled() const
{
    return m_inlineHookContext.bIsEnabled;
}

bool InlineHook::Is64Bit() const
{
    return m_inlineHookContext.bIs64Bit;
}

void InlineHook::SetEnabled(bool enabled)
{
}

void* InlineHook::GetTrampolineAddress() const
{
    return m_inlineHookContext.pTrampolineAddress;
}

const std::wstring& InlineHook::GetTargetModule() const
{
    return m_inlineHookContext.wstrTargetModule;
}

const std::string& InlineHook::GetTargetFunction() const
{
    return m_inlineHookContext.strTargetFuncName;
}

const std::wstring& InlineHook::GetHookType() const
{
    return L"InlineHook";
}

void* InlineHook::GetOriginalFunctionAddress() const
{
    void* ptr = nullptr;
    // 获取目标函数地址
    if (nullptr != m_inlineHookContext.pTargetAddress)
    {
        return m_inlineHookContext.pTargetAddress;
    }

    HMODULE hModule = GetModuleHandleW(m_inlineHookContext.wstrTargetModule.c_str());
    if (nullptr == hModule)
    {
        Logger::GetInstance().Error(L"GetModuleHandleW failed for module %s! error = %d", 
            m_inlineHookContext.wstrTargetModule.c_str(), GetLastError());
        return nullptr;
    }
    ptr = GetProcAddress(hModule, m_inlineHookContext.strTargetFuncName.c_str());
    if (nullptr == ptr)
    {
        Logger::GetInstance().Error(L"GetProcAddress failed for function %s! error = %d",
            m_inlineHookContext.strTargetFuncName.c_str(), GetLastError());
        return nullptr;
    }
    return ptr;
}

bool InlineHook::CreateTrampolineFunc()
{
    bool bRes = false;
    if (m_inlineHookContext.bIs64Bit)
    {
        bRes = Create64BitTrampolineFunc();
    }
    else
    {
        bRes = Create32BitTrampolineFunc();
    }
    return bRes;
}

bool InlineHook::Create32BitTrampolineFunc()
{
    if (nullptr == m_inlineHookContext.pTargetAddress)
    {
        Logger::GetInstance().Error(L"pTargetAddress is nullptr!");
        return false;
    }

    // 解析指令
    InstructionInfo_study info;
    if (FALSE == InstructionParser::ParseInstruction((BYTE*)m_inlineHookContext.pTargetAddress, &info, InstructionArchitecture::ARCH_X86))
    {
        Logger::GetInstance().Error(L"ParseInstruction failed!");
        return false;
    }

    // 记录指令长度
    m_inlineHookContext.sizePatch = info.length;
    //m_inlineHookContext.sizePatch = 6;

    // 申请一块内存，用于放置跳板函数，内存大小为指令长度 + 新的jmp指令
    m_inlineHookContext.pTrampolineAddress = VirtualAlloc(nullptr, m_inlineHookContext.sizePatch * 2, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (nullptr == m_inlineHookContext.pTrampolineAddress)
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    // 保存原始字节
    memcpy(m_inlineHookContext.byteOriginal, m_inlineHookContext.pTargetAddress, m_inlineHookContext.sizePatch);

    // 将原始字节先写入到申请内存中
    memcpy(m_inlineHookContext.pTrampolineAddress, m_inlineHookContext.byteOriginal, m_inlineHookContext.sizePatch);

    // 将原函数开头指令做重定向处理后写入跳板函数开头
    UINT uLen = 0;
    if (FALSE == InstructionRelocator::RelocateInstruction(&info, (BYTE*)((UINT_PTR)m_inlineHookContext.pTrampolineAddress), &uLen, InstructionArchitecture::ARCH_X86))
    {
        Logger::GetInstance().Error(L"RelocateInstruction failed!");
        // 释放内存
        if (FALSE == VirtualFree(m_inlineHookContext.pTrampolineAddress, m_inlineHookContext.sizePatch * 2, MEM_RELEASE))
        {
            Logger::GetInstance().Error(L"VirtualFree pTrampolineAddress failed! error = %d", GetLastError());
        }
        m_inlineHookContext.pTrampolineAddress = nullptr;
        return false;
    }

    if (info.length > 16)
    {
        Logger::GetInstance().Error(L"Instruction is too long!");
        // 释放内存
        if (FALSE == VirtualFree(m_inlineHookContext.pTrampolineAddress, m_inlineHookContext.sizePatch * 2, MEM_RELEASE))
        {
            Logger::GetInstance().Error(L"VirtualFree pTrampolineAddress failed! error = %d", GetLastError());
        }
        m_inlineHookContext.pTrampolineAddress = nullptr;
        return false;
    }

    // 在跳板函数末尾添加jmp回原函数位置的指令
    // 计算覆盖数据之后的原函数地址 (原函数地址+jmp指令长度)
    // uintptr_t专门用于保存指针地址，支持+-操作，根据系统自适应调整大小
    // 这里后续要进行的是+—操作进行地址计算，而不是读写内存
    // 所以使用uintptr_t，以确保语义清晰
    uintptr_t pJumpTarget = (uintptr_t)m_inlineHookContext.pTargetAddress + m_inlineHookContext.sizePatch;

    // 获取分配的内存中写入jmp指令的起始地址
    // 这里使用unsigned char*，这是一块将要进行读写操作的内存地址
    unsigned char* pJmpPtr = (unsigned char*)m_inlineHookContext.pTrampolineAddress + m_inlineHookContext.sizePatch;

    // 计算偏移
    DWORD dwOffset = pJumpTarget - ((uintptr_t)pJmpPtr + LEN_JUMP_BYTE_32);

    // 将jmp指令写入内存
    *pJmpPtr = 0xE9;
    memcpy(pJmpPtr + 1, &dwOffset, LEN_JUMP_BYTE_32 - 1);

    return true;
}

bool InlineHook::Create64BitTrampolineFunc()
{
    return false;
}

void InlineHook::FreeTrampolineFunc()
{
    if (nullptr != m_inlineHookContext.pTrampolineAddress)
    {
        if (FALSE == VirtualFree(m_inlineHookContext.pTrampolineAddress, m_inlineHookContext.sizePatch * 2, MEM_RELEASE))
        {
            Logger::GetInstance().Error(L"VirtualFree failed! error = %d", GetLastError());
        }
    }
}
