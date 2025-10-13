#pragma once

#include "IHook.h"

#include <Windows.h>

// IATHOOK ����Ҫ�����κ�״̬�����Բ���Ҫ�ض��Ĺ��캯��
class IATHook : public IHook
{
public:
    // ��ʼ������
    virtual bool Init(const Param& params) override;

    // ��װ Hook
    virtual bool Install() override;

    // ж�� Hook
    virtual bool Uninstall() override;

    // ��� Hook �Ƿ��Ѱ�װ
    virtual bool IsInstalled() const override;

    void* GetOriginalFuncAddress() const;

private:
    PVOID m_pOriginalFunction; // ԭ������ַ

    bool m_bIsInstalled;
    void* m_pHookFunction;             // Hook������ַ
    std::wstring m_wstrTargetModule;   // Ŀ��ģ��·��
    std::string m_strTargetFuncName;   // Ŀ�꺯������
    bool m_bIs64Bit;
};

