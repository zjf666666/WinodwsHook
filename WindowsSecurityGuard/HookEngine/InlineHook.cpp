#include "pch.h"
#include "InlineHook.h"
#include "../SecurityCore/Logger.h"
#include "../SecurityCore/ProcessUtils.h"

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
    DWORD dwOldProtectFlag;
    if (FALSE == VirtualProtect(m_inlineHookContext.pTargetAddress, m_inlineHookContext.sizePatch, PAGE_EXECUTE_READWRITE, &dwOldProtectFlag))
    {
        Logger::GetInstance().Error(L"Inline hook param error!");
        FreeTrampolineFunc();
        return false;
    }

    // д��jmpָ��
    unsigned char* pTargetPtr = (unsigned char*)m_inlineHookContext.pTargetAddress;
    *pTargetPtr = 0xE9;

    // ����ƫ��
    DWORD dwOffset = (uintptr_t)m_inlineHookContext.pHookFunction - ((uintptr_t)pTargetPtr + m_inlineHookContext.sizePatch);

    // ��ƫ��д���ڴ�
    memcpy(pTargetPtr + 1, &dwOffset, m_inlineHookContext.sizePatch - 1);

    m_inlineHookContext.bIsInstalled = true;

    // �ָ��ڴ�ҳ������
    if (FALSE == VirtualProtect(m_inlineHookContext.pTargetAddress, m_inlineHookContext.sizePatch, dwOldProtectFlag, &dwOldProtectFlag))
    {
        // ���ﲻ���ͷ��κ���Դ����Ϊ�Ѿ�д��ȥ�ˣ�ֻ���ͷ�������
        Logger::GetInstance().Error(L"Inline hook param error!");
        return false;
    }
    return true;
}

bool InlineHook::Uninstall()
{
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
    // ��ȡԭ������ַ
    return nullptr;
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
    m_inlineHookContext.sizePatch = LEN_JUMP_BYTE_32;
    if (nullptr == m_inlineHookContext.pTargetAddress)
    {
        Logger::GetInstance().Error(L"pTargetAddress is nullptr!");
        return false;
    }

    // ����ԭʼ�ֽ�
    memcpy(m_inlineHookContext.byteOriginal, m_inlineHookContext.pTargetAddress, m_inlineHookContext.sizePatch);

    // ����һ���ڴ棬���ڷ������庯�����ڴ��СΪ�滻�ַ�����5�� + �µ�jmpָ�5��
    m_inlineHookContext.pTrampolineAddress = VirtualAlloc(nullptr, m_inlineHookContext.sizePatch * 2 , MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (nullptr == m_inlineHookContext.pTrampolineAddress)
    {
        Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
        return false;
    }

    // ��ԭʼ�ֽ���д�뵽�����ڴ���
    memcpy(m_inlineHookContext.pTrampolineAddress, m_inlineHookContext.byteOriginal, m_inlineHookContext.sizePatch);

    // ���㸲������֮���ԭ������ַ (ԭ������ַ+jmpָ���)
    // uintptr_tר�����ڱ���ָ���ַ��֧��+-����������ϵͳ����Ӧ������С
    // �������Ҫ���е���+���������е�ַ���㣬�����Ƕ�д�ڴ�
    // ����ʹ��uintptr_t����ȷ����������
    uintptr_t pJumpTarget = (uintptr_t)m_inlineHookContext.pTargetAddress + m_inlineHookContext.sizePatch;

    // ��ȡ������ڴ���д��jmpָ�����ʼ��ַ
    // ����ʹ��unsigned char*������һ�齫Ҫ���ж�д�������ڴ��ַ
    unsigned char* pJmpPtr = (unsigned char*)m_inlineHookContext.pTrampolineAddress + m_inlineHookContext.sizePatch;

    // ����ƫ��
    DWORD dwOffset = pJumpTarget - ((uintptr_t)pJmpPtr + m_inlineHookContext.sizePatch);

    // ��jmpָ��д���ڴ�
    *pJmpPtr = 0xE9;
    memcpy(pJmpPtr + 1, &dwOffset, m_inlineHookContext.sizePatch - 1);

    return true;
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
