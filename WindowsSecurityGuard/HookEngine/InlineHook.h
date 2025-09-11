#pragma once

#include "IHook.h"

class InlineHook : public IHook
{
public:
    InlineHook(const std::wstring& targetModule, const std::string& targetFunction, void* hookFunction);

    bool Install() override;

    bool Uninstall() override;

    bool IsInstalled() const override;

    const std::wstring& GetTargetModule() const override;

    const std::string& GetTargetFunction() const override;

    const std::wstring& GetHookType() const override;

protected:
    void* GetOriginalFunctionAddress() const override;

private:
    bool m_bIsInstalled;             // �Ƿ��Ѱ�װ
    std::wstring m_wstrTargetModule; // ģ������
    std::string m_strTargetFuncName;     // ��������
    void* m_pHookFunction;            // Hook ����ָ��
    void* m_pOriginalFunction;        // ԭʼ����ָ��
    unsigned char m_originalBytes[16]; // ԭʼ�ֽ�
};

