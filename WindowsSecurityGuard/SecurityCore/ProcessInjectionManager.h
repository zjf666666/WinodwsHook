#pragma once

#include <vector>
#include <string>

#include <Windows.h>

#include "IInjectionStrategy.h"

enum class InjectionMethod {
    CreateRemoteThread,    // ʹ��CreateRemoteThread API
    QueueUserAPC,         // ʹ��QueueUserAPC API
    SetWindowsHookEx,     // ʹ��SetWindowsHookEx API
    ThreadHijacking       // �߳̽ٳ�
};

// ����ע��������࣬�ṩ����ӿڣ�ֻ������Ľӿڿ���ֱ�Ӹ��ⲿҵ�����
struct InjectionResult {
    bool Success;                  // ע���Ƿ�ɹ�
    std::wstring ErrorMessage;    // ������Ϣ
    DWORD ErrorCode;              // �������
    DWORD InjectedModuleBase;     // ע��ģ���ַ
    InjectionResult() : Success(false), ErrorCode(0), InjectedModuleBase(0) {}
};

class ProcessInjectionManager {
public:
    // �������ʵ�
    static ProcessInjectionManager& GetInstance();

    // ע�����
    InjectionResult InjectDll(DWORD processId, const std::wstring& dllPath, InjectionMethod method = InjectionMethod::CreateRemoteThread);
    InjectionResult EjectDll(DWORD processId, const std::wstring& dllPath);
    
    /* Ԥ���ӿڣ�Զ��ע�������� */
    //InjectionResult InjectDll(const IInjectionTarget& target, const std::wstring& dllPath, InjectionMethod method = InjectionMethod::CreateRemoteThread);
    //InjectionResult EjectDll(const IInjectionTarget& target, const std::wstring& dllPath);

private:
    // ����ע�����
    bool CreateInjectStrategy(InjectionMethod method);

    IInjectionStrategy* GetStrategy(InjectionMethod method);

private:
    // ˽�й������������������ģʽ��
    ProcessInjectionManager();
    ~ProcessInjectionManager();

    // ���ÿ�������͸�ֵ����
    ProcessInjectionManager(const ProcessInjectionManager&) = delete;
    ProcessInjectionManager& operator=(const ProcessInjectionManager&) = delete;
};

