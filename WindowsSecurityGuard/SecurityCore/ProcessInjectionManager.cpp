#include "pch.h"
#include "ProcessInjectionManager.h"
#include "InjectionStrategies.h"
#include "Logger.h"

ProcessInjectionManager& ProcessInjectionManager::GetInstance()
{
    static ProcessInjectionManager inject;
    return inject;
}

bool ProcessInjectionManager::InjectDll(DWORD pid, const std::wstring& dllPath, InjectionMethod method)
{
    if (method >= InjectionMethod::ERROR_METHOD || method < InjectionMethod::CreateRemoteThread)
    {
        Logger::GetInstance().Error(L"method: %d out of range!", (int)method);
        return false;
    }

    int nSize = m_vecInjectionStrategy.size();
    if (nSize < (size_t)method)
    {
        // ʹ��ѭ����ֵ�ķ�ʽ����vector������ʹ��resizeʱ��unique_ptr���п����Ĵ������
        for (size_t i = nSize; i < (size_t)method + 1; ++i)
        {
            m_vecInjectionStrategy.emplace_back(nullptr);
        }
    }

    // ����У��ȷ��ע�����ָ�벻Ϊ�գ����Ϊ�գ�����
    if (nullptr == m_vecInjectionStrategy[(size_t)method])
    {
        CreateInjectStrategy(method);
        if (nullptr == m_vecInjectionStrategy[(size_t)method])
        {
            Logger::GetInstance().Error(L"CreateInjectStrategy failed");
            return false;
        }
    }

    bool bRes = m_vecInjectionStrategy[(size_t)method]->Inject(pid, dllPath);
    return bRes;
}

void ProcessInjectionManager::CreateInjectStrategy(InjectionMethod method)
{
    switch (method) // ʹ�ù���ģʽ��̬��������
    {
    case InjectionMethod::CreateRemoteThread:
        m_vecInjectionStrategy[(size_t)method] = std::make_unique<CreateRemoteThreadStrategy>();
        break;
    case InjectionMethod::QueueUserAPC:
        //inject = new QueueUserAPCStrategy();
        break;
    case InjectionMethod::SetWindowsHookExW:
        break;
    case InjectionMethod::ThreadHijacking:
        break;
    default:
        break;
    }
}

IInjectionStrategy* ProcessInjectionManager::GetStrategy(InjectionMethod method)
{
    return nullptr;
}

ProcessInjectionManager::ProcessInjectionManager()
{
    // ʹ��ѭ����ֵ�ķ�ʽ����vector������ʹ��resizeʱ��unique_ptr���п����Ĵ������
    for (size_t i = 0; i < SIZE_INJECTION_METHOD; ++i)
    {
        m_vecInjectionStrategy.emplace_back(nullptr);
    }
}

ProcessInjectionManager::~ProcessInjectionManager()
{
}
