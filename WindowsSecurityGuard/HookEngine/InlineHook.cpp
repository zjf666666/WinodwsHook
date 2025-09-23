#include "pch.h"
#include "InlineHook.h"

#include "../SecurityCore/Logger.h"
#include "../SecurityCore/ProcessUtils.h"
#include "../SecurityCore/VirtualMemoryWrapper.h"
#include "../SecurityCore/HandleWrapper.h"
#include "ZydisUtils.h"

#define LEN_JUMP_BYTE_32   5      // 32位进程jmp指令长度
#define MAX_LEN_ALLOC_32   17     // 32位进程最多需要使用的指令长度
#define MAX_LEN_ALLOC_64   0x100  // 64位进程需要多申请一些内存

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
    VirtualProtectWrapper virtualWrapper(m_inlineHookContext.pTargetAddress, m_inlineHookContext.sizeParse, PAGE_EXECUTE_READWRITE);
    if (!virtualWrapper.IsValid())
    {
        Logger::GetInstance().Error(L"Inline hook param error!");
        FreeTrampolineFunc();
        return false;
    }

    // 创建覆盖原函数指令
    CreateCoverInst();

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
    ZydisContextPtr zyData = ZydisUtils::CreateContext(false);
    size_t sizeParseLen = 0;
    if (FALSE == ZydisUtils::ParseInstruction((BYTE*)m_inlineHookContext.pTargetAddress, 16, &m_inlineHookContext.sizeParse, zyData))
    {
        Logger::GetInstance().Error(L"ParseInstruction failed!");
        return false;
    }

    // 申请一块内存，用于放置跳板函数，内存大小为指令长度 + 新的jmp指令
    m_inlineHookContext.pTrampolineAddress = VirtualAlloc(nullptr, MAX_LEN_ALLOC_32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (nullptr == m_inlineHookContext.pTrampolineAddress)
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    // 保存原始字节
    memcpy(m_inlineHookContext.byteOriginal, m_inlineHookContext.pTargetAddress, m_inlineHookContext.sizeParse);

    // 将原始字节先写入到申请内存中
    memcpy(m_inlineHookContext.pTrampolineAddress, m_inlineHookContext.byteOriginal, m_inlineHookContext.sizeParse);

    // 将原函数开头指令做重定向处理后写入跳板函数开头
    SIZE_T uLen = 0, uBufferSize = MAX_LEN_ALLOC_32;
    if (FALSE == ZydisUtils::RelocateInstruction(zyData, (BYTE*)m_inlineHookContext.pTargetAddress, (BYTE*)((UINT_PTR)m_inlineHookContext.pTrampolineAddress), &uBufferSize, &uLen))
    {
        Logger::GetInstance().Error(L"RelocateInstruction failed!");
        // 释放内存
        if (FALSE == VirtualFree(m_inlineHookContext.pTrampolineAddress, MAX_LEN_ALLOC_32, MEM_RELEASE))
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
    // 原函数地址+解析指令长度
    uintptr_t pJumpTarget = (uintptr_t)m_inlineHookContext.pTargetAddress + m_inlineHookContext.sizeParse;

    // 获取分配的内存中写入jmp指令的起始地址
    // 这里使用unsigned char*，这是一块将要进行读写操作的内存地址
    // 跳板函数起点 + 重定向后的指令长度
    unsigned char* pJmpPtr = (unsigned char*)m_inlineHookContext.pTrampolineAddress + uLen;

    // 计算偏移
    DWORD dwOffset = pJumpTarget - ((uintptr_t)pJmpPtr + LEN_JUMP_BYTE_32);

    // 将jmp指令写入内存
    *pJmpPtr = 0xE9;
    memcpy(pJmpPtr + 1, &dwOffset, LEN_JUMP_BYTE_32 - 1);

    return true;
}

bool InlineHook::Create64BitTrampolineFunc()
{
    if (nullptr == m_inlineHookContext.pTargetAddress)
    {
        Logger::GetInstance().Error(L"pTargetAddress is nullptr!");
        return false;
    }

    // 申请一块内存，用于放置跳板函数，内存大小为指令长度 + 新的jmp指令
    m_inlineHookContext.pTrampolineAddress = VirtualAlloc(nullptr, MAX_LEN_ALLOC_64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (nullptr == m_inlineHookContext.pTrampolineAddress)
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    ZydisContextPtr zyData = ZydisUtils::CreateContext(true);
    m_inlineHookContext.sizeParse = 0;
    SIZE_T uTotalRelocateLen = 0;
    while (m_inlineHookContext.sizeParse < 12) // 至少需要解析12个字节的指令
    {
        // 解析指令
        SIZE_T sizeParseLen = 0;
        if (FALSE == ZydisUtils::ParseInstruction((BYTE*)((UINT_PTR)m_inlineHookContext.pTargetAddress + m_inlineHookContext.sizeParse), 16 - m_inlineHookContext.sizeParse, &sizeParseLen, zyData))
        {
            Logger::GetInstance().Error(L"ParseInstruction failed!");
            return false;
        }
        m_inlineHookContext.sizeParse += sizeParseLen;

        // 重定向指令
        // 将原函数开头指令做重定向处理后写入跳板函数开头
        SIZE_T uBufferSize = MAX_LEN_ALLOC_64 - uTotalRelocateLen, uRelocationLen = 0;;
        if (FALSE == ZydisUtils::RelocateInstruction(zyData, (BYTE*)((UINT_PTR)m_inlineHookContext.pTargetAddress + m_inlineHookContext.sizeParse - sizeParseLen), (BYTE*)((UINT_PTR)m_inlineHookContext.pTrampolineAddress + uTotalRelocateLen), &uBufferSize, &uRelocationLen))
        {
            Logger::GetInstance().Error(L"RelocateInstruction failed!");
            // 释放内存
            if (FALSE == VirtualFree(m_inlineHookContext.pTrampolineAddress, MAX_LEN_ALLOC_64, MEM_RELEASE))
            {
                Logger::GetInstance().Error(L"VirtualFree pTrampolineAddress failed! error = %d", GetLastError());
            }
            m_inlineHookContext.pTrampolineAddress = nullptr;
            return false;
        }
        uTotalRelocateLen += uRelocationLen;
    }

    // 保存原始字节
    memcpy(m_inlineHookContext.byteOriginal, m_inlineHookContext.pTargetAddress, m_inlineHookContext.sizeParse);

    // 将原始字节先写入到申请内存中
    // memcpy(m_inlineHookContext.pTrampolineAddress, m_inlineHookContext.byteOriginal, m_inlineHookContext.sizeParse);

    // 在跳板函数末尾添加jmp回原函数位置的指令
    // 计算覆盖数据之后的原函数地址 (原函数地址+jmp指令长度)
    // uintptr_t专门用于保存指针地址，支持+-操作，根据系统自适应调整大小
    // 这里后续要进行的是+—操作进行地址计算，而不是读写内存
    // 所以使用uintptr_t，以确保语义清晰
    // 原函数地址+解析指令长度
    uintptr_t pJumpTarget = (uintptr_t)m_inlineHookContext.pTargetAddress + m_inlineHookContext.sizeParse;

    // 获取分配的内存中写入jmp指令的起始地址
    // 这里使用unsigned char*，这是一块将要进行读写操作的内存地址
    // 跳板函数起点 + 重定向后的指令长度
    unsigned char* pJmpPtr = (unsigned char*)m_inlineHookContext.pTrampolineAddress + uTotalRelocateLen;

    // 计算偏移
    DWORD dwOffset = pJumpTarget - ((uintptr_t)pJmpPtr + 12);

    // 将jmp指令写入内存
    pJmpPtr[0] = 0x48;  // REX.W
    pJmpPtr[1] = 0xB8;  // MOV RAX, imm64
    memcpy(pJmpPtr + 2, &pJumpTarget, sizeof(pJumpTarget));
    pJmpPtr[10] = 0xFF; // JMP RAX
    pJmpPtr[11] = 0xE0;

    return true;
}

void InlineHook::FreeTrampolineFunc()
{
    if (nullptr != m_inlineHookContext.pTrampolineAddress)
    {
        if (FALSE == VirtualFree(m_inlineHookContext.pTrampolineAddress, m_inlineHookContext.sizeParse * 2, MEM_RELEASE))
        {
            Logger::GetInstance().Error(L"VirtualFree failed! error = %d", GetLastError());
        }
    }
}

bool InlineHook::CreateCoverInst()
{
    bool bRes = false;
    if (m_inlineHookContext.bIs64Bit)
    {
        bRes = Create64BitCoverInst();
    }
    else
    {
        bRes = Create32BitCoverInst();
    }
    return bRes;
}

bool InlineHook::Create32BitCoverInst()
{
    unsigned char* pTargetPtr = (unsigned char*)m_inlineHookContext.pTargetAddress;
    *pTargetPtr = 0xE9;

    // 计算偏移
    DWORD dwOffset = (uintptr_t)m_inlineHookContext.pHookFunction - ((uintptr_t)pTargetPtr + LEN_JUMP_BYTE_32);

    // 将偏移写入内存
    memcpy(pTargetPtr + 1, &dwOffset, LEN_JUMP_BYTE_32 - 1);

    return true;
}

bool InlineHook::Create64BitCoverInst()
{
    BYTE* targetAddress = (BYTE*)m_inlineHookContext.pTargetAddress;
    UINT_PTR absoluteAddr = (UINT_PTR)m_inlineHookContext.pHookFunction;

    targetAddress[0] = 0x48;  // REX.W
    targetAddress[1] = 0xB8;  // MOV RAX, imm64
    memcpy(targetAddress + 2, &absoluteAddr, sizeof(absoluteAddr));
    targetAddress[10] = 0xFF; // JMP RAX
    targetAddress[11] = 0xE0;

    return true;
}
