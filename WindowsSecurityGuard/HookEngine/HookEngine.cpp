#include "pch.h"
#include "HookEngine.h"
#include "HookFactory.h"
#include "HookIdFactory.h"
#include "../SecurityCore/Logger.h"
#include "../SecurityCore/OpenSSLUtils.h"

#define ALGO_VERSION   "1"

HookEngine::HookEngine()
{

}

HookEngine::~HookEngine()
{

}

HookEngine& HookEngine::GetInstance()
{
    static HookEngine hookEngine;
    return hookEngine;
}

bool HookEngine::StartHook(HookType type, const HookParam& param)
{
    // 计算HOOKID
    HookIdInfo* info = new HookIdInfo();
    HookId hookId = GenerateHookId(type, param, info);

    // 使用unique_ptr接管info对象指针，避免后续忘记释放
    std::unique_ptr<HookIdInfo> uniqueInfo;
    uniqueInfo.reset(info);

    // HOOKID是否已存在
    std::lock_guard<std::mutex> lock(m_mtxHookInfo);
    if (m_mapHookInfo.find(hookId) != m_mapHookInfo.end())
    {
        Logger::GetInstance().Info(L"Hook is already exists.");
        return true; // 已安装返回true
    }

    // 根据类型创建hook类
    IHook* hook = HookFactory::CreateHook(type);
    if (nullptr == hook)
    {
        Logger::GetInstance().Error(L"Create hook failed!");
        return false;
    }

    // 由unique_ptr接手IHook对象指针，避免后续忘记释放
    uniqueInfo->hook.reset(hook);

    // 初始化hook类
    if (false == uniqueInfo->hook->Init(param))
    {
        Logger::GetInstance().Error(L"Init hook failed!");
        return false;
    }

    // 安装hook类
    if (false == uniqueInfo->hook->Install())
    {
        Logger::GetInstance().Error(L"Install hook failed!");
        return false;
    }

    m_mapHookInfo[hookId] = std::move(uniqueInfo);
    return true;
}

bool HookEngine::StopHook(HookId id)
{
    std::lock_guard<std::mutex> lock(m_mtxHookInfo);
    if (m_mapHookInfo.find(id) == m_mapHookInfo.end())
    {
        Logger::GetInstance().Info(L"Hook is already uninstall.");
        return true; // 没有找到 默认已卸载 返回成功
    }

    if (false == m_mapHookInfo[id]->hook->Uninstall())
    {
        Logger::GetInstance().Error(L"Uninstall failed!");
        return false;
    }
    return true;
}

void HookEngine::RegisterObserver(IHookObserver* observer)
{
    if (nullptr == observer)
    {
        Logger::GetInstance().Error(L"observer is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock(m_mtxObserver);
    std::vector<IHookObserver*>::iterator iter = std::find(m_vecObserver.begin(), m_vecObserver.end(), observer);
    if (iter != m_vecObserver.end())
    {
        Logger::GetInstance().Info(L"observer is already register.");
        return;
    }
    m_vecObserver.push_back(observer);
}

void HookEngine::UnregisterObserver(IHookObserver* observer)
{
    if (nullptr == observer)
    {
        Logger::GetInstance().Error(L"observer is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock(m_mtxObserver);
    std::vector<IHookObserver*>::iterator iter = std::find(m_vecObserver.begin(), m_vecObserver.end(), observer);
    if (iter == m_vecObserver.end())
    {
        Logger::GetInstance().Info(L"observer is already unregister.");
        return;
    }
    m_vecObserver.erase(iter);
}

std::string HookEngine::GenerateHookId(HookType type, const HookParam& param, HookIdInfo* info)
{
    IHookId* hookId = HookIdFactory::CreateHookId(type);
    if (nullptr == hookId)
    {
        Logger::GetInstance().Error(L"Create hookid failed!");
        return "";
    }
    // 格式 v{版本}:{哈希}
    std::string strRes = std::string("v{") + ALGO_VERSION + std::string("}:{");
    std::string strKey = hookId->GenerateKey(param, info);
    strRes += CalculateHash(strKey);
    strRes += "}";
}

std::string HookEngine::CalculateHash(const std::string& strKey)
{
    // 这里封装的原因是，减少加密算法与原函数的耦合度
    return OpenSSLUtils::Sha256(strKey);
}
