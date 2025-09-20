#pragma once

#include <Windows.h>

#include "IHook.h"


// inline hook���ݽṹ��
struct InlineHookContext : public BaseHookContext
{
    void* pHookFunction;             // Hook������ַ
    void* pTrampolineAddress;        // ���庯����ַ һ�鵥������Ŀɶ���д�ڴ棬�����˱��滻��ԭʼ�ֽ����� + ����ԭ�����������ݵ�ָ��
    BYTE byteOriginal[16];           // ԭʼ�ֽ�
    SIZE_T sizePatch;                // ������С x86 5�ֽڣ� x64 12+�ֽ�
};

class InlineHook : public IHook
{
public:
    InlineHook(
        const std::wstring& targetModule,
        const std::string& targetFunction,
        void* hookFunction, // ����ʵ�ֵ�hook������ַ
        bool bIs64Bit
    );

    bool Install() override;

    bool Uninstall() override;

    bool IsInstalled() const override;

    bool IsEnabled() const override;

    bool Is64Bit() const override;

    void SetEnabled(bool enabled) override;

    void* GetTrampolineAddress() const;

    const std::wstring& GetTargetModule() const override;

    const std::string& GetTargetFunction() const override;

    const std::wstring& GetHookType() const override;

protected:
    void* GetOriginalFunctionAddress() const override;

private:
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

private:
    InlineHookContext m_inlineHookContext;
};

