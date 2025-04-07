#pragma once

/*******************************************************
 * @auth : zhengjunfeng                                *
 * @time : 2023-12-04                                  *
 * @brief: inline hook. ʹ��jmp 5λ�ķ�ʽ               *
 *         ��ȡ�����ĵ�ַ��ʹ�û���е�jmp��䣬          *
 *         ��ԭ��Ӧ�ý���ĺ�������޸�ΪHOOK�������      *
 *******************************************************/

#include <windows.h>

#define LEN_JUMP_BYTE   5

class inlineHook
{
public:
    inlineHook();
    ~inlineHook();
    
    void installMyHook();   // ����HOOK����

    void unistallMyHookByAddr();  // ж��HOOK����

    void unistallMyHookByName();  // ж��HOOK����

    void reInstallMyHookByAddr(); // ���¼���HOOK����

    void reInstallMyHookByName(); // ���¼���HOOK����

    /*
     * @brief HOOK���� ͨ��GetProcAddress������ȡhook������ǰ��λ���� Ȼ���滻
     * @param [in] pszModuleName ��Ҫhook�ĺ������ڵ�ģ��
     *        [in] pszFunctionName ��Ҫhook�ĺ�������
     *        [in] procMyFunctionAddr hook��������Ҫ��ת�ĺ���ָ��
     */
    bool myHookByAddr(LPCWSTR pszModuleName, LPCSTR pszFunctionName, DWORD procMyFunctionAddr);

    /*
     * @brief HOOK���� ͨ��(char*)������ ��ȡhook������ǰ��λ���� Ȼ���滻
     * @param [in] pszHookFunc hook�ĺ������׵�ַ
     *        [in] procMyFunctionAddr hook��������Ҫ��ת�ĺ���ָ��
     */
    bool myHookByName(LPSTR pszHookFunc, DWORD procMyFunctionAddr);

private:
    FARPROC m_procHookFunAddr;      // PROC = INT* ��¼HOOKǰ�ĺ�����ַ
    char* m_procHookFunName;
    BYTE m_btOrigin[LEN_JUMP_BYTE]; // ��¼ת��Ϊjmp���ǰLEN_JUMP_BYTEλ�Ļ������
    BYTE m_btMyJump[LEN_JUMP_BYTE]; // ����HOOK���LEN_JUMP_BYTEλ�Ļ�����
};

