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
        // 使用循环赋值的方式扩充vector，避免使用resize时对unique_ptr进行拷贝的错误操作
        for (size_t i = nSize; i < (size_t)method + 1; ++i)
        {
            m_vecInjectionStrategy.emplace_back(nullptr);
        }
    }

    // 二次校验确保注入策略指针不为空，如果为空，报错
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
    switch (method) // 使用工厂模式动态创建策略
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
    // 使用循环赋值的方式扩充vector，避免使用resize时对unique_ptr进行拷贝的错误操作
    for (size_t i = 0; i < SIZE_INJECTION_METHOD; ++i)
    {
        m_vecInjectionStrategy.emplace_back(nullptr);
    }
}

ProcessInjectionManager::~ProcessInjectionManager()
{
}
