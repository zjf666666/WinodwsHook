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

bool InlineHook::Init(const HookParam& params)
{
    auto strArch = params.Get<std::string>("architecture");
    auto hookFunc = params.Get<void*>("hook function address");
    auto targetFunc = params.Get<std::string>("target function name");
    auto targetModule = params.Get<const WCHAR>("target module name");
    if (!strArch || !hookFunc || !targetFunc || !targetModule)
    {
        Logger::GetInstance().Error(L"Get params failed!");
        return false;
    }

    m_bIs64Bit = *strArch == "x64" ? true : false;
    m_bIsInstalled = false; // 这个参数后续需要在内部做检测，而不是外部传入
    m_pHookFunction = *hookFunc;
    m_pTargetAddress = nullptr;
    m_pTrampolineAddress = nullptr;
    m_strTargetFuncName = *targetFunc;
    m_wstrTargetModule = *targetModule;
    return true;
}

bool InlineHook::Install()
{
    // 检测参数是否合法
    if (m_bIsInstalled)
    {
        // 如果已安装 返回true，记录info日志
        Logger::GetInstance().Info(L"Inline hook has already installed");
        return true;
    }

    if (m_wstrTargetModule.empty() || m_strTargetFuncName.empty() || nullptr == m_pHookFunction)
    {
        // 模块路径、函数名称、hook函数地址三个数据均不能为空
        Logger::GetInstance().Error(L"Inline hook param error!");
        return false;
    }

    // 获取原函数地址并保存
    m_pTargetAddress = GetOriginalFunctionAddress();

    // 填充跳板函数
    CreateTrampolineFunc();

    // 修改内存页状态为可读可写
    VirtualProtectWrapper virtualWrapper(m_pTargetAddress, m_sizeParse, PAGE_EXECUTE_READWRITE);
    if (!virtualWrapper.IsValid())
    {
        Logger::GetInstance().Error(L"Inline hook param error!");
        FreeTrampolineFunc();
        return false;
    }

    // 创建覆盖原函数指令
    CreateCoverInst();

    m_bIsInstalled = true;

    // virtualWrapper 析构时自动还原内存保护属性
    return true;
}

bool InlineHook::Uninstall()
{
    // 还原指令
    if (nullptr == m_pTargetAddress)
    {
        Logger::GetInstance().Error(L"Target address is nullptr!");
        return false;
    }

    VirtualProtectWrapper virtualWrapper(m_pTargetAddress, m_sizeParse, PAGE_EXECUTE_READWRITE);
    if (!virtualWrapper.IsValid())
    {
        Logger::GetInstance().Error(L"Change virtual protect failed! error = %d", GetLastError());
        FreeTrampolineFunc();
        return false;
    }

    memcpy(m_pTargetAddress, m_byteOriginal, m_sizeParse);
    FreeTrampolineFunc(); // 释放跳板函数
    return true;
}

bool InlineHook::IsInstalled() const
{
    return m_bIsInstalled;
}

void* InlineHook::GetTrampolineAddress() const
{
    return m_pTrampolineAddress;
}

void* InlineHook::GetOriginalFunctionAddress() const
{
    void* ptr = nullptr;
    // 获取目标函数地址
    if (nullptr != m_pTargetAddress)
    {
        return m_pTargetAddress;
    }

    HMODULE hModule = GetModuleHandleW(m_wstrTargetModule.c_str());
    if (nullptr == hModule)
    {
        Logger::GetInstance().Error(L"GetModuleHandleW failed for module %s! error = %d", 
            m_wstrTargetModule.c_str(), GetLastError());
        return nullptr;
    }
    ptr = GetProcAddress(hModule, m_strTargetFuncName.c_str());
    if (nullptr == ptr)
    {
        Logger::GetInstance().Error(L"GetProcAddress failed for function %s! error = %d",
            m_strTargetFuncName.c_str(), GetLastError());
        return nullptr;
    }
    return ptr;
}

bool InlineHook::CreateTrampolineFunc()
{
    bool bRes = false;
    if (m_bIs64Bit)
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
    if (nullptr == m_pTargetAddress)
    {
        Logger::GetInstance().Error(L"pTargetAddress is nullptr!");
        return false;
    }

    // 解析指令
    ZydisContextPtr zyData = ZydisUtils::CreateContext(false);
    size_t sizeParseLen = 0;
    if (FALSE == ZydisUtils::ParseInstruction((BYTE*)m_pTargetAddress, 16, &m_sizeParse, zyData))
    {
        Logger::GetInstance().Error(L"ParseInstruction failed!");
        return false;
    }

    // 申请一块内存，用于放置跳板函数，内存大小为指令长度 + 新的jmp指令
    m_pTrampolineAddress = VirtualAlloc(nullptr, MAX_LEN_ALLOC_32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (nullptr == m_pTrampolineAddress)
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    // 保存原始字节
    memcpy(m_byteOriginal, m_pTargetAddress, m_sizeParse);

    // 将原始字节先写入到申请内存中
    memcpy(m_pTrampolineAddress, m_byteOriginal, m_sizeParse);

    // 将原函数开头指令做重定向处理后写入跳板函数开头
    SIZE_T uLen = 0, uBufferSize = MAX_LEN_ALLOC_32;
    if (FALSE == ZydisUtils::RelocateInstruction(zyData, (BYTE*)m_pTargetAddress, (BYTE*)((UINT_PTR)m_pTrampolineAddress), &uBufferSize, &uLen))
    {
        Logger::GetInstance().Error(L"RelocateInstruction failed!");
        // 释放内存
        if (FALSE == VirtualFree(m_pTrampolineAddress, MAX_LEN_ALLOC_32, MEM_RELEASE))
        {
            Logger::GetInstance().Error(L"VirtualFree pTrampolineAddress failed! error = %d", GetLastError());
        }
        m_pTrampolineAddress = nullptr;
        return false;
    }

    // 在跳板函数末尾添加jmp回原函数位置的指令
    // 计算覆盖数据之后的原函数地址 (原函数地址+jmp指令长度)
    // uintptr_t专门用于保存指针地址，支持+-操作，根据系统自适应调整大小
    // 这里后续要进行的是+—操作进行地址计算，而不是读写内存
    // 所以使用uintptr_t，以确保语义清晰
    // 原函数地址+解析指令长度
    uintptr_t pJumpTarget = (uintptr_t)m_pTargetAddress + m_sizeParse;

    // 获取分配的内存中写入jmp指令的起始地址
    // 这里使用unsigned char*，这是一块将要进行读写操作的内存地址
    // 跳板函数起点 + 重定向后的指令长度
    unsigned char* pJmpPtr = (unsigned char*)m_pTrampolineAddress + uLen;

    // 计算偏移
    DWORD dwOffset = pJumpTarget - ((uintptr_t)pJmpPtr + LEN_JUMP_BYTE_32);

    // 将jmp指令写入内存
    *pJmpPtr = 0xE9;
    memcpy(pJmpPtr + 1, &dwOffset, LEN_JUMP_BYTE_32 - 1);

    return true;
}

bool InlineHook::Create64BitTrampolineFunc()
{
    if (nullptr == m_pTargetAddress)
    {
        Logger::GetInstance().Error(L"pTargetAddress is nullptr!");
        return false;
    }

    // 申请一块内存，用于放置跳板函数，内存大小为指令长度 + 新的jmp指令
    m_pTrampolineAddress = VirtualAlloc(nullptr, MAX_LEN_ALLOC_64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (nullptr == m_pTrampolineAddress)
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    ZydisContextPtr zyData = ZydisUtils::CreateContext(true);
    m_sizeParse = 0;
    SIZE_T uTotalRelocateLen = 0;
    while (m_sizeParse < 12) // 至少需要解析12个字节的指令
    {
        // 解析指令
        SIZE_T sizeParseLen = 0;
        if (FALSE == ZydisUtils::ParseInstruction((BYTE*)((UINT_PTR)m_pTargetAddress + m_sizeParse), 16 - m_sizeParse, &sizeParseLen, zyData))
        {
            Logger::GetInstance().Error(L"ParseInstruction failed!");
            return false;
        }
        m_sizeParse += sizeParseLen;

        // 重定向指令
        // 将原函数开头指令做重定向处理后写入跳板函数开头
        SIZE_T uBufferSize = MAX_LEN_ALLOC_64 - uTotalRelocateLen, uRelocationLen = 0;;
        if (FALSE == ZydisUtils::RelocateInstruction(zyData, (BYTE*)((UINT_PTR)m_pTargetAddress + m_sizeParse - sizeParseLen), (BYTE*)((UINT_PTR)m_pTrampolineAddress + uTotalRelocateLen), &uBufferSize, &uRelocationLen))
        {
            Logger::GetInstance().Error(L"RelocateInstruction failed!");
            // 释放内存
            if (FALSE == VirtualFree(m_pTrampolineAddress, MAX_LEN_ALLOC_64, MEM_RELEASE))
            {
                Logger::GetInstance().Error(L"VirtualFree pTrampolineAddress failed! error = %d", GetLastError());
            }
            m_pTrampolineAddress = nullptr;
            return false;
        }
        uTotalRelocateLen += uRelocationLen;
    }

    // 保存原始字节
    memcpy(m_byteOriginal, m_pTargetAddress, m_sizeParse);

    // 将原始字节先写入到申请内存中
    // memcpy(.pTrampolineAddress, .byteOriginal, .sizeParse);

    // 在跳板函数末尾添加jmp回原函数位置的指令
    // 计算覆盖数据之后的原函数地址 (原函数地址+jmp指令长度)
    // uintptr_t专门用于保存指针地址，支持+-操作，根据系统自适应调整大小
    // 这里后续要进行的是+—操作进行地址计算，而不是读写内存
    // 所以使用uintptr_t，以确保语义清晰
    // 原函数地址+解析指令长度
    uintptr_t pJumpTarget = (uintptr_t)m_pTargetAddress + m_sizeParse;

    // 获取分配的内存中写入jmp指令的起始地址
    // 这里使用unsigned char*，这是一块将要进行读写操作的内存地址
    // 跳板函数起点 + 重定向后的指令长度
    unsigned char* pJmpPtr = (unsigned char*)m_pTrampolineAddress + uTotalRelocateLen;

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
    if (nullptr != m_pTrampolineAddress)
    {
        if (FALSE == VirtualFree(m_pTrampolineAddress, m_sizeParse * 2, MEM_RELEASE))
        {
            Logger::GetInstance().Error(L"VirtualFree failed! error = %d", GetLastError());
        }
    }
}

bool InlineHook::CreateCoverInst()
{
    bool bRes = false;
    if (m_bIs64Bit)
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
    unsigned char* pTargetPtr = (unsigned char*)m_pTargetAddress;
    *pTargetPtr = 0xE9;

    // 计算偏移
    DWORD dwOffset = (uintptr_t)m_pHookFunction - ((uintptr_t)pTargetPtr + LEN_JUMP_BYTE_32);

    // 将偏移写入内存
    memcpy(pTargetPtr + 1, &dwOffset, LEN_JUMP_BYTE_32 - 1);

    return true;
}

bool InlineHook::Create64BitCoverInst()
{
    BYTE* targetAddress = (BYTE*)m_pTargetAddress;
    UINT_PTR absoluteAddr = (UINT_PTR)m_pHookFunction;

    targetAddress[0] = 0x48;  // REX.W
    targetAddress[1] = 0xB8;  // MOV RAX, imm64
    memcpy(targetAddress + 2, &absoluteAddr, sizeof(absoluteAddr));
    targetAddress[10] = 0xFF; // JMP RAX
    targetAddress[11] = 0xE0;

    return true;
}
