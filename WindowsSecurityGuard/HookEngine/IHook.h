#pragma once

#include <string>

// ͨ��HOOK���ݽṹ��
struct BaseHookContext
{
    bool bIsInstalled;               // �Ƿ��Ѱ�װ
    bool bIsEnabled;                 // �Ƿ����Hook����
    bool bIs64Bit;                   // �Ƿ�Ϊ64Ϊ����
    void* pTargetAddress;            // ��HOOK�ĺ�����ַ
    std::wstring wstrTargetModule;   // Ŀ��ģ��·��
    std::string strTargetFuncName;   // Ŀ�꺯������
};

/* Hook �ӿ��࣬�������� Hook ʵ�ֵ�ͨ�ýӿ� */
class IHook
{
public:
    ~IHook() = default; // �������ִ�C++���ϰ�� ��ͬ�� ~IHook() {}

    // ��װ Hook
    virtual bool Install() = 0;

    // ж�� Hook
    virtual bool Uninstall() = 0;

    // ��� Hook �Ƿ��Ѱ�װ
    virtual bool IsInstalled() const = 0;

    // Hook�Ƿ����
    virtual bool IsEnabled() const = 0;

    // �Ƿ���64λ�ܹ�
    virtual bool Is64Bit() const = 0;

    // ���ÿ�����
    virtual void SetEnabled(bool enabled) = 0;

    // ��ȡԭʼ����
    template<typename T>
    T GetOriginalFunction() {
        return reinterpret_cast<T>(GetOriginalFunctionAddress());
    }

    // ��ȡĿ��ģ����
    // ���ﷵ��ֵ��wstring����Ϊģ���·�����ƣ�DLL��exe��ͨ����Ҫ֧��Unicode�ַ�
    // ����LoadLibraryW��GetModuleHandleW��
    virtual const std::wstring& GetTargetModule() const = 0;

    // ��ȡĿ�꺯����
    // ���ﷵ��ֵ��string����Ϊ����������windows��һ������ASCII���ŵģ���PEͷ�У�ʹ�õ���ASCII�����Unicode�ַ�
    // ����GetProcAddress����ֻ֧��string���룬�������ﷵ��string
    virtual const std::string& GetTargetFunction() const = 0;

    // ��ȡ Hook ����
    virtual const std::wstring& GetHookType() const = 0;

protected:
    // ��ȡԭʼ������ַ
    virtual void* GetOriginalFunctionAddress() const = 0;
};

