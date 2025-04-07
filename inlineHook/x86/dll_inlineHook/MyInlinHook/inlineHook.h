#pragma once

/*******************************************************
 * @auth : zhengjunfeng                                *
 * @time : 2023-12-04                                  *
 * @brief: inline hook. 使用jmp 5位的方式               *
 *         获取函数的地址，使用汇编中的jmp语句，          *
 *         将原来应该进入的函数入口修改为HOOK函数入口      *
 *******************************************************/

#include <windows.h>

#define LEN_JUMP_BYTE   5

class inlineHook
{
public:
    inlineHook();
    ~inlineHook();
    
    void installMyHook();   // 加载HOOK函数

    void unistallMyHookByAddr();  // 卸载HOOK函数

    void unistallMyHookByName();  // 卸载HOOK函数

    void reInstallMyHookByAddr(); // 重新加载HOOK函数

    void reInstallMyHookByName(); // 重新加载HOOK函数

    /*
     * @brief HOOK函数 通过GetProcAddress函数获取hook函数的前几位数据 然后替换
     * @param [in] pszModuleName 需要hook的函数所在的模块
     *        [in] pszFunctionName 需要hook的函数名称
     *        [in] procMyFunctionAddr hook到函数后要跳转的函数指针
     */
    bool myHookByAddr(LPCWSTR pszModuleName, LPCSTR pszFunctionName, DWORD procMyFunctionAddr);

    /*
     * @brief HOOK函数 通过(char*)函数名 获取hook函数的前几位数据 然后替换
     * @param [in] pszHookFunc hook的函数的首地址
     *        [in] procMyFunctionAddr hook到函数后要跳转的函数指针
     */
    bool myHookByName(LPSTR pszHookFunc, DWORD procMyFunctionAddr);

private:
    FARPROC m_procHookFunAddr;      // PROC = INT* 记录HOOK前的函数地址
    char* m_procHookFunName;
    BYTE m_btOrigin[LEN_JUMP_BYTE]; // 记录转换为jmp语句前LEN_JUMP_BYTE位的汇编内容
    BYTE m_btMyJump[LEN_JUMP_BYTE]; // 我们HOOK后的LEN_JUMP_BYTE位的汇编语句
};

