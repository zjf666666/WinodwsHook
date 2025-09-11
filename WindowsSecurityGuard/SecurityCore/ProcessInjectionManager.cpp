#include "pch.h"
#include "ProcessInjectionManager.h"
#include "InjectionStrategies.h"

ProcessInjectionManager& ProcessInjectionManager::GetInstance()
{
    static ProcessInjectionManager inject;
    return inject;
}

bool ProcessInjectionManager::CreateInjectStrategy(InjectionMethod method)
{
    IInjectionStrategy* inject = NULL;
    switch (method) // ʹ�ù���ģʽ��̬��������
    {
    case InjectionMethod::CreateRemoteThread:
        inject = new CreateRemoteThreadStrategy();
        break;
    case InjectionMethod::QueueUserAPC:
        inject = new QueueUserAPCStrategy();
        break;
    case InjectionMethod::SetWindowsHookExW:
        break;
    case InjectionMethod::ThreadHijacking:
        break;
    default:
        break;
    }
    return inject;
}

InjectionResult ProcessInjectionManager::InjectDll(DWORD processId, const std::wstring& dllPath, InjectionMethod method)
{
    return InjectionResult();
}

