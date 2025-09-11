#pragma once

#include <vector>
#include <string>
#include <memory>

#include <Windows.h>

#include "IInjectionStrategy.h"

#define SIZE_INJECTION_METHOD   4

enum class InjectionMethod {
    CreateRemoteThread,    // ʹ��CreateRemoteThread API
    QueueUserAPC,         // ʹ��QueueUserAPC API
    SetWindowsHookEx,     // ʹ��SetWindowsHookEx API
    ThreadHijacking,       // �߳̽ٳ�
    ERROR_METHOD          // �����ж�method�Ƿ�Ϸ�
};

// �������ʱ����
struct InjectionResult {
    DWORD ErrorCode;              // �������
    DWORD InjectedModuleBase;     // ע��ģ���ַ
    InjectionResult() : ErrorCode(0), InjectedModuleBase(0) {}
};

// ����ע��������࣬�ṩ����ӿڣ�ֻ������Ľӿڿ���ֱ�Ӹ��ⲿҵ�����
class ProcessInjectionManager {
public:
    // �������ʵ�
    static ProcessInjectionManager& GetInstance();

    // ע�����
    bool InjectDll(DWORD pid, const std::wstring& dllPath, InjectionMethod method = InjectionMethod::CreateRemoteThread);
    bool EjectDll(DWORD pid, const std::wstring& dllPath);
    
    /* Ԥ���ӿڣ�Զ��ע�������� */
    //InjectionResult InjectDll(const IInjectionTarget& target, const std::wstring& dllPath, InjectionMethod method = InjectionMethod::CreateRemoteThread);
    //InjectionResult EjectDll(const IInjectionTarget& target, const std::wstring& dllPath);

private:
    // ����ע����ԣ��������ֻ���𴴽����ж��߼������ⲿ����
    void CreateInjectStrategy(InjectionMethod method);

    IInjectionStrategy* GetStrategy(InjectionMethod method);

private:
    // ˽�й������������������ģʽ��
    ProcessInjectionManager();
    ~ProcessInjectionManager();

    // ���ÿ�������͸�ֵ����
    ProcessInjectionManager(const ProcessInjectionManager&) = delete;
    ProcessInjectionManager& operator=(const ProcessInjectionManager&) = delete;

private:
    std::vector<std::unique_ptr<IInjectionStrategy>> m_vecInjectionStrategy;
};

