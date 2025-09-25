#include "pch.h"
#include "InlineHook.h"

#include "../SecurityCore/Logger.h"
#include "../SecurityCore/ProcessUtils.h"
#include "../SecurityCore/VirtualMemoryWrapper.h"
#include "../SecurityCore/HandleWrapper.h"
#include "ZydisUtils.h"

#define LEN_JUMP_BYTE_32   5      // 32λ����jmpָ���
#define MAX_LEN_ALLOC_32   17     // 32λ���������Ҫʹ�õ�ָ���
#define MAX_LEN_ALLOC_64   0x100  // 64λ������Ҫ������һЩ�ڴ�

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
    m_bIsInstalled = false; // �������������Ҫ���ڲ�����⣬�������ⲿ����
    m_pHookFunction = *hookFunc;
    m_pTargetAddress = nullptr;
    m_pTrampolineAddress = nullptr;
    m_strTargetFuncName = *targetFunc;
    m_wstrTargetModule = *targetModule;
    return true;
}

bool InlineHook::Install()
{
    // �������Ƿ�Ϸ�
    if (m_bIsInstalled)
    {
        // ����Ѱ�װ ����true����¼info��־
        Logger::GetInstance().Info(L"Inline hook has already installed");
        return true;
    }

    if (m_wstrTargetModule.empty() || m_strTargetFuncName.empty() || nullptr == m_pHookFunction)
    {
        // ģ��·�����������ơ�hook������ַ�������ݾ�����Ϊ��
        Logger::GetInstance().Error(L"Inline hook param error!");
        return false;
    }

    // ��ȡԭ������ַ������
    m_pTargetAddress = GetOriginalFunctionAddress();

    // ������庯��
    CreateTrampolineFunc();

    // �޸��ڴ�ҳ״̬Ϊ�ɶ���д
    VirtualProtectWrapper virtualWrapper(m_pTargetAddress, m_sizeParse, PAGE_EXECUTE_READWRITE);
    if (!virtualWrapper.IsValid())
    {
        Logger::GetInstance().Error(L"Inline hook param error!");
        FreeTrampolineFunc();
        return false;
    }

    // ��������ԭ����ָ��
    CreateCoverInst();

    m_bIsInstalled = true;

    // virtualWrapper ����ʱ�Զ���ԭ�ڴ汣������
    return true;
}

bool InlineHook::Uninstall()
{
    // ��ԭָ��
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
    FreeTrampolineFunc(); // �ͷ����庯��
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
    // ��ȡĿ�꺯����ַ
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

    // ����ָ��
    ZydisContextPtr zyData = ZydisUtils::CreateContext(false);
    size_t sizeParseLen = 0;
    if (FALSE == ZydisUtils::ParseInstruction((BYTE*)m_pTargetAddress, 16, &m_sizeParse, zyData))
    {
        Logger::GetInstance().Error(L"ParseInstruction failed!");
        return false;
    }

    // ����һ���ڴ棬���ڷ������庯�����ڴ��СΪָ��� + �µ�jmpָ��
    m_pTrampolineAddress = VirtualAlloc(nullptr, MAX_LEN_ALLOC_32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (nullptr == m_pTrampolineAddress)
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    // ����ԭʼ�ֽ�
    memcpy(m_byteOriginal, m_pTargetAddress, m_sizeParse);

    // ��ԭʼ�ֽ���д�뵽�����ڴ���
    memcpy(m_pTrampolineAddress, m_byteOriginal, m_sizeParse);

    // ��ԭ������ͷָ�����ض������д�����庯����ͷ
    SIZE_T uLen = 0, uBufferSize = MAX_LEN_ALLOC_32;
    if (FALSE == ZydisUtils::RelocateInstruction(zyData, (BYTE*)m_pTargetAddress, (BYTE*)((UINT_PTR)m_pTrampolineAddress), &uBufferSize, &uLen))
    {
        Logger::GetInstance().Error(L"RelocateInstruction failed!");
        // �ͷ��ڴ�
        if (FALSE == VirtualFree(m_pTrampolineAddress, MAX_LEN_ALLOC_32, MEM_RELEASE))
        {
            Logger::GetInstance().Error(L"VirtualFree pTrampolineAddress failed! error = %d", GetLastError());
        }
        m_pTrampolineAddress = nullptr;
        return false;
    }

    // �����庯��ĩβ���jmp��ԭ����λ�õ�ָ��
    // ���㸲������֮���ԭ������ַ (ԭ������ַ+jmpָ���)
    // uintptr_tר�����ڱ���ָ���ַ��֧��+-����������ϵͳ����Ӧ������С
    // �������Ҫ���е���+���������е�ַ���㣬�����Ƕ�д�ڴ�
    // ����ʹ��uintptr_t����ȷ����������
    // ԭ������ַ+����ָ���
    uintptr_t pJumpTarget = (uintptr_t)m_pTargetAddress + m_sizeParse;

    // ��ȡ������ڴ���д��jmpָ�����ʼ��ַ
    // ����ʹ��unsigned char*������һ�齫Ҫ���ж�д�������ڴ��ַ
    // ���庯����� + �ض�����ָ���
    unsigned char* pJmpPtr = (unsigned char*)m_pTrampolineAddress + uLen;

    // ����ƫ��
    DWORD dwOffset = pJumpTarget - ((uintptr_t)pJmpPtr + LEN_JUMP_BYTE_32);

    // ��jmpָ��д���ڴ�
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

    // ����һ���ڴ棬���ڷ������庯�����ڴ��СΪָ��� + �µ�jmpָ��
    m_pTrampolineAddress = VirtualAlloc(nullptr, MAX_LEN_ALLOC_64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (nullptr == m_pTrampolineAddress)
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    ZydisContextPtr zyData = ZydisUtils::CreateContext(true);
    m_sizeParse = 0;
    SIZE_T uTotalRelocateLen = 0;
    while (m_sizeParse < 12) // ������Ҫ����12���ֽڵ�ָ��
    {
        // ����ָ��
        SIZE_T sizeParseLen = 0;
        if (FALSE == ZydisUtils::ParseInstruction((BYTE*)((UINT_PTR)m_pTargetAddress + m_sizeParse), 16 - m_sizeParse, &sizeParseLen, zyData))
        {
            Logger::GetInstance().Error(L"ParseInstruction failed!");
            return false;
        }
        m_sizeParse += sizeParseLen;

        // �ض���ָ��
        // ��ԭ������ͷָ�����ض������д�����庯����ͷ
        SIZE_T uBufferSize = MAX_LEN_ALLOC_64 - uTotalRelocateLen, uRelocationLen = 0;;
        if (FALSE == ZydisUtils::RelocateInstruction(zyData, (BYTE*)((UINT_PTR)m_pTargetAddress + m_sizeParse - sizeParseLen), (BYTE*)((UINT_PTR)m_pTrampolineAddress + uTotalRelocateLen), &uBufferSize, &uRelocationLen))
        {
            Logger::GetInstance().Error(L"RelocateInstruction failed!");
            // �ͷ��ڴ�
            if (FALSE == VirtualFree(m_pTrampolineAddress, MAX_LEN_ALLOC_64, MEM_RELEASE))
            {
                Logger::GetInstance().Error(L"VirtualFree pTrampolineAddress failed! error = %d", GetLastError());
            }
            m_pTrampolineAddress = nullptr;
            return false;
        }
        uTotalRelocateLen += uRelocationLen;
    }

    // ����ԭʼ�ֽ�
    memcpy(m_byteOriginal, m_pTargetAddress, m_sizeParse);

    // ��ԭʼ�ֽ���д�뵽�����ڴ���
    // memcpy(.pTrampolineAddress, .byteOriginal, .sizeParse);

    // �����庯��ĩβ���jmp��ԭ����λ�õ�ָ��
    // ���㸲������֮���ԭ������ַ (ԭ������ַ+jmpָ���)
    // uintptr_tר�����ڱ���ָ���ַ��֧��+-����������ϵͳ����Ӧ������С
    // �������Ҫ���е���+���������е�ַ���㣬�����Ƕ�д�ڴ�
    // ����ʹ��uintptr_t����ȷ����������
    // ԭ������ַ+����ָ���
    uintptr_t pJumpTarget = (uintptr_t)m_pTargetAddress + m_sizeParse;

    // ��ȡ������ڴ���д��jmpָ�����ʼ��ַ
    // ����ʹ��unsigned char*������һ�齫Ҫ���ж�д�������ڴ��ַ
    // ���庯����� + �ض�����ָ���
    unsigned char* pJmpPtr = (unsigned char*)m_pTrampolineAddress + uTotalRelocateLen;

    // ����ƫ��
    DWORD dwOffset = pJumpTarget - ((uintptr_t)pJmpPtr + 12);

    // ��jmpָ��д���ڴ�
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

    // ����ƫ��
    DWORD dwOffset = (uintptr_t)m_pHookFunction - ((uintptr_t)pTargetPtr + LEN_JUMP_BYTE_32);

    // ��ƫ��д���ڴ�
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
