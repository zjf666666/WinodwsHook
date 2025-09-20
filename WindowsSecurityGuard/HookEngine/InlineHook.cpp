#include "pch.h"
#include "InlineHook.h"

#include "../SecurityCore/Logger.h"
#include "../SecurityCore/ProcessUtils.h"
#include "../SecurityCore/VirtualMemoryWrapper.h"
#include "../SecurityCore//HandleWrapper.h"
#include "InstructionParser.h"
#include "InstructionRelocator.h"

#define LEN_JUMP_BYTE_32   5  // 32λ����jmpָ���

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
    // �������Ƿ�Ϸ�
    if (m_inlineHookContext.bIsInstalled)
    {
        // ����Ѱ�װ ����true����¼info��־
        Logger::GetInstance().Info(L"Inline hook has already installed");
        return true;
    }

    if (m_inlineHookContext.wstrTargetModule.empty() ||
        m_inlineHookContext.strTargetFuncName.empty() ||
        nullptr == m_inlineHookContext.pHookFunction)
    {
        // ģ��·�����������ơ�hook������ַ�������ݾ�����Ϊ��
        Logger::GetInstance().Error(L"Inline hook param error!");
        return false;
    }

    // ��ȡԭ������ַ������
    m_inlineHookContext.pTargetAddress = GetOriginalFunctionAddress();

    // ������庯��
    CreateTrampolineFunc();

    // �޸��ڴ�ҳ״̬Ϊ�ɶ���д
    VirtualProtectWrapper virtualWrapper(m_inlineHookContext.pTargetAddress, m_inlineHookContext.sizePatch, PAGE_EXECUTE_READWRITE);
    if (!virtualWrapper.IsValid())
    {
        Logger::GetInstance().Error(L"Inline hook param error!");
        FreeTrampolineFunc();
        return false;
    }

    // д��jmpָ��
    unsigned char* pTargetPtr = (unsigned char*)m_inlineHookContext.pTargetAddress;
    *pTargetPtr = 0xE9;

    // ����ƫ��
    DWORD dwOffset = (uintptr_t)m_inlineHookContext.pHookFunction - ((uintptr_t)pTargetPtr + LEN_JUMP_BYTE_32);

    // ��ƫ��д���ڴ�
    memcpy(pTargetPtr + 1, &dwOffset, LEN_JUMP_BYTE_32 - 1);

    m_inlineHookContext.bIsInstalled = true;

    // virtualWrapper ����ʱ�Զ���ԭ�ڴ汣������
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
    // ��ȡĿ�꺯����ַ
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

    // ����ָ��
    InstructionInfo_study info;
    if (FALSE == InstructionParser::ParseInstruction((BYTE*)m_inlineHookContext.pTargetAddress, &info, InstructionArchitecture::ARCH_X86))
    {
        Logger::GetInstance().Error(L"ParseInstruction failed!");
        return false;
    }

    // ��¼ָ���
    m_inlineHookContext.sizePatch = info.length;
    //m_inlineHookContext.sizePatch = 6;

    // ����һ���ڴ棬���ڷ������庯�����ڴ��СΪָ��� + �µ�jmpָ��
    m_inlineHookContext.pTrampolineAddress = VirtualAlloc(nullptr, m_inlineHookContext.sizePatch * 2, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (nullptr == m_inlineHookContext.pTrampolineAddress)
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    // ����ԭʼ�ֽ�
    memcpy(m_inlineHookContext.byteOriginal, m_inlineHookContext.pTargetAddress, m_inlineHookContext.sizePatch);

    // ��ԭʼ�ֽ���д�뵽�����ڴ���
    memcpy(m_inlineHookContext.pTrampolineAddress, m_inlineHookContext.byteOriginal, m_inlineHookContext.sizePatch);

    // ��ԭ������ͷָ�����ض������д�����庯����ͷ
    UINT uLen = 0;
    if (FALSE == InstructionRelocator::RelocateInstruction(&info, (BYTE*)((UINT_PTR)m_inlineHookContext.pTrampolineAddress), &uLen, InstructionArchitecture::ARCH_X86))
    {
        Logger::GetInstance().Error(L"RelocateInstruction failed!");
        // �ͷ��ڴ�
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
        // �ͷ��ڴ�
        if (FALSE == VirtualFree(m_inlineHookContext.pTrampolineAddress, m_inlineHookContext.sizePatch * 2, MEM_RELEASE))
        {
            Logger::GetInstance().Error(L"VirtualFree pTrampolineAddress failed! error = %d", GetLastError());
        }
        m_inlineHookContext.pTrampolineAddress = nullptr;
        return false;
    }

    // �����庯��ĩβ���jmp��ԭ����λ�õ�ָ��
    // ���㸲������֮���ԭ������ַ (ԭ������ַ+jmpָ���)
    // uintptr_tר�����ڱ���ָ���ַ��֧��+-����������ϵͳ����Ӧ������С
    // �������Ҫ���е���+���������е�ַ���㣬�����Ƕ�д�ڴ�
    // ����ʹ��uintptr_t����ȷ����������
    uintptr_t pJumpTarget = (uintptr_t)m_inlineHookContext.pTargetAddress + m_inlineHookContext.sizePatch;

    // ��ȡ������ڴ���д��jmpָ�����ʼ��ַ
    // ����ʹ��unsigned char*������һ�齫Ҫ���ж�д�������ڴ��ַ
    unsigned char* pJmpPtr = (unsigned char*)m_inlineHookContext.pTrampolineAddress + m_inlineHookContext.sizePatch;

    // ����ƫ��
    DWORD dwOffset = pJumpTarget - ((uintptr_t)pJmpPtr + LEN_JUMP_BYTE_32);

    // ��jmpָ��д���ڴ�
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
