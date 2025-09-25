#pragma once

#include <Windows.h>

#include "IHook.h"

class InlineHook : public IHook
{
public:
    bool Init(const HookParam& params) override;

    bool Install() override;

    bool Uninstall() override;

    bool IsInstalled() const override;

    void* GetTrampolineAddress() const;

private:
    void* GetOriginalFunctionAddress() const;

    // �������庯��
    /*
     * ƫ�Ƽ��㹫ʽ��ƫ�� = Ŀ�꺯����ַ - ����ǰ������ַ + ָ��ȣ�
     * CPUָ�ִ�з�ʽ��
     * 1. CPU��ȡ������jmpָ���ʱָ��ָ���Ѿ��ƶ�����һ��ָ����׵�ַ��
     * 2. CPU������Ҫ��ת�ĵ�ַ
     * 3. CPU�������ַ���ȼӵ�ָ��ָ���ϵõ�Ҫ��ת�ĺ�����ַ
     * ��� jmpָ�����ڵ�ַ + ָ��� + ƫ�Ƴ��� = Ŀ�꺯����ַ
     * jmpָ����Դ�����ַ����ˣ����ǲ�����������ֽ������ǵ��ֽ���
     */
    bool CreateTrampolineFunc();

    bool Create32BitTrampolineFunc();

    bool Create64BitTrampolineFunc();

    void FreeTrampolineFunc();

    bool CreateCoverInst();

    bool Create32BitCoverInst();

    bool Create64BitCoverInst();

private:
    bool m_bIsInstalled;               // �Ƿ��Ѱ�װ
    bool m_bIs64Bit;                   // �Ƿ�Ϊ64Ϊ����
    void* m_pTargetAddress;            // ��HOOK�ĺ�����ַ
    std::wstring m_wstrTargetModule;   // Ŀ��ģ��·��
    std::string m_strTargetFuncName;   // Ŀ�꺯������

    void* m_pHookFunction;             // Hook������ַ
    void* m_pTrampolineAddress;        // ���庯����ַ һ�鵥������Ŀɶ���д�ڴ棬�����˱��滻��ԭʼ�ֽ����� + ����ԭ�����������ݵ�ָ��
    BYTE m_byteOriginal[16];           // ԭʼ�ֽ�
    SIZE_T m_sizeParse;                // ������С x86 5�ֽڣ� x64 12+�ֽ�
};

