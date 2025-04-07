#include "inlineHook.h"
#include "../../../interface/logRecord.h"

inlineHook::inlineHook()
{
    m_procHookFunAddr = NULL;
    m_procHookFunName = NULL;
    memset(m_btOrigin, 0, LEN_JUMP_BYTE);
    memset(m_btMyJump, 0, LEN_JUMP_BYTE);
    m_btMyJump[0] = 0xE9; // 第一位是jmp
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
        if (!WriteProcessMemory(hProcess, m_procHookFunAddr, m_btOrigin, LEN_JUMP_BYTE, NULL)) //把原函数的前5位数据写回去
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
        VirtualProtect((char*)m_procHookFunName, LEN_JUMP_BYTE, dwOld, &dwOld); // 还原原来的进程属性
    }
}

void inlineHook::reInstallMyHookByAddr()
{
    if (NULL != m_procHookFunAddr)
    {
        HANDLE hProcess = GetCurrentProcess();
        if (!WriteProcessMemory(hProcess, m_procHookFunAddr, m_btMyJump, LEN_JUMP_BYTE, NULL)) // 把hook后函数的偏移写到被hook函数的前5位地址
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
    if (!VirtualProtect(m_procHookFunName, LEN_JUMP_BYTE, PAGE_EXECUTE_READWRITE, &dwOld)) // 改变原先的进程读写属性为可读可写并保留原来的进程属性到dwOld里
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"VirtualProtect failed, error = ", L"%d", GetLastError());
        return;
    }
    logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_INFO, L"VirtualProtect");
    memcpy(m_procHookFunName, m_btMyJump, LEN_JUMP_BYTE); // 保存注入前的数据
    VirtualProtect(m_procHookFunName, LEN_JUMP_BYTE, dwOld, &dwOld); // 还原原来的进程属性
}

bool inlineHook::myHookByAddr(LPCWSTR pszModuleName, LPCSTR pszFunctionName, DWORD procMyFunctionAddr)
{
    logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_INFO, L"MY HOOK");
    HMODULE hModule = LoadLibrary(pszModuleName); // 获取模块句柄
    if (NULL == hModule)
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"LoadLibrary failed, error = ", L"%d", GetLastError());
        return false;
    }

    logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_INFO, L"LoadLibrary");
    m_procHookFunAddr = GetProcAddress(hModule, pszFunctionName); // 获取模块中函数地址
    FARPROC addr = GetProcAddress(hModule, pszFunctionName);

    if (NULL == m_procHookFunAddr)
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"GetProcAddress failed, error = ", L"%d", GetLastError());
        FreeLibrary(hModule);
        return false;
    }

    HANDLE hProcess = GetCurrentProcess(); // 这个句柄是个伪句柄，不需要使用closehandle函数释放
    if (NULL == hProcess)
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"GetCurrentProcess failed, error = ", L"%d", GetLastError());
        FreeLibrary(hModule);
        return false;
    }
    if (!ReadProcessMemory(hProcess, m_procHookFunAddr, m_btOrigin, LEN_JUMP_BYTE, NULL)) // 读取被HOOK函数的前5位数据用于还原
    { 
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"ReadProcessMemory failed, error = ", L"%d", GetLastError());
    }
    logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_INFO, L"ReadProcessMemory");

    /*
        jmp到这个偏移是因为：执行到jmp指令时，会先进行取值（把这条jmp指令传入cpu），之后地址移动到下一条指令
        然后由控制器执行jmp指令，发现是jmp指令，会根据当前指向的指令来计算真实地址（当前指向指令位置+jmp跟着的偏移地址）
        所以jmp跟着的偏移地址是 当前函数地址+5+offset = 要跳转到的地址
    */
    DWORD dwOffset = procMyFunctionAddr - (DWORD)m_procHookFunAddr - LEN_JUMP_BYTE; // 计算偏移
    memcpy(&m_btMyJump[1], &dwOffset, LEN_JUMP_BYTE - 1); // jmp 地址

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
    memcpy(m_btOrigin, m_procHookFunName, LEN_JUMP_BYTE); // 原地址数据
    DWORD dwOffset = procMyFunctionAddr - (DWORD)m_procHookFunName - LEN_JUMP_BYTE; // 计算偏移
    memcpy(&m_btMyJump[1], &dwOffset, LEN_JUMP_BYTE - 1); // jmp 地址

    DWORD dwOld = 0;
    if (!VirtualProtect(m_procHookFunName, LEN_JUMP_BYTE, PAGE_EXECUTE_READWRITE, &dwOld)) // 改变原先的进程读写属性为可读可写并保留原来的进程属性到dwOld里
    {
        logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_ERROR, L"VirtualProtect failed, error = ", L"%d", GetLastError());
        return false;
    }
    logRecord::getInstance()->writeMsg(LOGTYPE::LEVEL_INFO, L"VirtualProtect");
    memcpy(m_procHookFunName, m_btMyJump, LEN_JUMP_BYTE); // 保存注入前的数据
    VirtualProtect(m_procHookFunName, LEN_JUMP_BYTE, dwOld, &dwOld); // 还原原来的进程属性
    return true;
}



