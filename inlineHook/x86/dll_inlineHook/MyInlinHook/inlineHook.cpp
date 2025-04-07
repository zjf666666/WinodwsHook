#include "inlineHook.h"
#include "../../../interface/logRecord.h"

inlineHook::inlineHook()
{
    m_procHookFunAddr = NULL;
    m_procHookFunName = NULL;
    memset(m_btOrigin, 0, LEN_JUMP_BYTE);
    memset(m_btMyJump, 0, LEN_JUMP_BYTE);
    m_btMyJump[0] = 0xE9; // ��һλ��jmp
}

inlineHook::~inlineHook()
{
    if (NULL != m_procHookFunAddr)
    {
        unistallMyHookByAddr();
    }
    if (NULL != m_procHookFunName)
    {
        unistallMyHookByName();
    }
    m_procHookFunAddr = NULL;
    m_procHookFunName = NULL;
    memset(m_btOrigin, 0, LEN_JUMP_BYTE);
    memset(m_btMyJump, 0, LEN_JUMP_BYTE);
}

void inlineHook::installMyHook()
{
    
}

void inlineHook::unistallMyHookByAddr()
{
    if (NULL != m_procHookFunAddr)
    {
        HANDLE hProcess = GetCurrentProcess();
        if (!WriteProcessMemory(hProcess, m_procHookFunAddr, m_btOrigin, LEN_JUMP_BYTE, NULL)) //��ԭ������ǰ5λ����д��ȥ
        {
            logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"WriteProcessMemory failed, error = ", L"%d", GetLastError());
        }
    }
}

void inlineHook::unistallMyHookByName()
{
    if (NULL != m_procHookFunName)
    {
        DWORD dwOld;
        if (!VirtualProtect((char*)m_procHookFunName, LEN_JUMP_BYTE, PAGE_EXECUTE_READWRITE, &dwOld))
        {
            logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"VirtualProtect failed, error = ", L"%d", GetLastError());
            return;
        }
        memcpy((char*)m_procHookFunName, m_btOrigin, LEN_JUMP_BYTE);
        VirtualProtect((char*)m_procHookFunName, LEN_JUMP_BYTE, dwOld, &dwOld); // ��ԭԭ���Ľ�������
    }
}

void inlineHook::reInstallMyHookByAddr()
{
    if (NULL != m_procHookFunAddr)
    {
        HANDLE hProcess = GetCurrentProcess();
        if (!WriteProcessMemory(hProcess, m_procHookFunAddr, m_btMyJump, LEN_JUMP_BYTE, NULL)) // ��hook������ƫ��д����hook������ǰ5λ��ַ
        {
            logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"WriteProcessMemory failed, error = ", L"%d", GetLastError());
        }
    }
}

void inlineHook::reInstallMyHookByName()
{
    if (NULL == m_procHookFunName)
    {
        return;
    }
    DWORD dwOld = 0;
    if (!VirtualProtect(m_procHookFunName, LEN_JUMP_BYTE, PAGE_EXECUTE_READWRITE, &dwOld)) // �ı�ԭ�ȵĽ��̶�д����Ϊ�ɶ���д������ԭ���Ľ������Ե�dwOld��
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"VirtualProtect failed, error = ", L"%d", GetLastError());
        return;
    }
    logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_INFO, L"VirtualProtect");
    memcpy(m_procHookFunName, m_btMyJump, LEN_JUMP_BYTE); // ����ע��ǰ������
    VirtualProtect(m_procHookFunName, LEN_JUMP_BYTE, dwOld, &dwOld); // ��ԭԭ���Ľ�������
}

bool inlineHook::myHookByAddr(LPCWSTR pszModuleName, LPCSTR pszFunctionName, DWORD procMyFunctionAddr)
{
    logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_INFO, L"MY HOOK");
    HMODULE hModule = LoadLibrary(pszModuleName); // ��ȡģ����
    if (NULL == hModule)
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"LoadLibrary failed, error = ", L"%d", GetLastError());
        return false;
    }

    logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_INFO, L"LoadLibrary");
    m_procHookFunAddr = GetProcAddress(hModule, pszFunctionName); // ��ȡģ���к�����ַ
    FARPROC addr = GetProcAddress(hModule, pszFunctionName);

    if (NULL == m_procHookFunAddr)
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"GetProcAddress failed, error = ", L"%d", GetLastError());
        FreeLibrary(hModule);
        return false;
    }

    HANDLE hProcess = GetCurrentProcess(); // �������Ǹ�α���������Ҫʹ��closehandle�����ͷ�
    if (NULL == hProcess)
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"GetCurrentProcess failed, error = ", L"%d", GetLastError());
        FreeLibrary(hModule);
        return false;
    }
    if (!ReadProcessMemory(hProcess, m_procHookFunAddr, m_btOrigin, LEN_JUMP_BYTE, NULL)) // ��ȡ��HOOK������ǰ5λ�������ڻ�ԭ
    { 
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"ReadProcessMemory failed, error = ", L"%d", GetLastError());
    }
    logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_INFO, L"ReadProcessMemory");

    /*
        jmp�����ƫ������Ϊ��ִ�е�jmpָ��ʱ�����Ƚ���ȡֵ��������jmpָ���cpu����֮���ַ�ƶ�����һ��ָ��
        Ȼ���ɿ�����ִ��jmpָ�������jmpָ�����ݵ�ǰָ���ָ����������ʵ��ַ����ǰָ��ָ��λ��+jmp���ŵ�ƫ�Ƶ�ַ��
        ����jmp���ŵ�ƫ�Ƶ�ַ�� ��ǰ������ַ+5+offset = Ҫ��ת���ĵ�ַ
    */
    DWORD dwOffset = procMyFunctionAddr - (DWORD)m_procHookFunAddr - LEN_JUMP_BYTE; // ����ƫ��
    memcpy(&m_btMyJump[1], &dwOffset, LEN_JUMP_BYTE - 1); // jmp ��ַ

    DWORD dwNum = 0;
    if (!WriteProcessMemory(hProcess, m_procHookFunAddr, m_btMyJump, LEN_JUMP_BYTE, NULL))
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"WriteProcessMemory failed, error = ", L"%d", GetLastError());
    }
    if (NULL != hModule)
    {
        FreeLibrary(hModule);
    }

    return true;
}

bool inlineHook::myHookByName(LPSTR pszHookFunc, DWORD procMyFunctionAddr)
{
    m_procHookFunName = pszHookFunc;
    memcpy(m_btOrigin, m_procHookFunName, LEN_JUMP_BYTE); // ԭ��ַ����
    DWORD dwOffset = procMyFunctionAddr - (DWORD)m_procHookFunName - LEN_JUMP_BYTE; // ����ƫ��
    memcpy(&m_btMyJump[1], &dwOffset, LEN_JUMP_BYTE - 1); // jmp ��ַ

    DWORD dwOld = 0;
    if (!VirtualProtect(m_procHookFunName, LEN_JUMP_BYTE, PAGE_EXECUTE_READWRITE, &dwOld)) // �ı�ԭ�ȵĽ��̶�д����Ϊ�ɶ���д������ԭ���Ľ������Ե�dwOld��
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"VirtualProtect failed, error = ", L"%d", GetLastError());
        return false;
    }
    logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_INFO, L"VirtualProtect");
    memcpy(m_procHookFunName, m_btMyJump, LEN_JUMP_BYTE); // ����ע��ǰ������
    VirtualProtect(m_procHookFunName, LEN_JUMP_BYTE, dwOld, &dwOld); // ��ԭԭ���Ľ�������
    return true;
}



