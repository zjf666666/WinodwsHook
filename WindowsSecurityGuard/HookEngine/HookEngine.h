#pragma once

#include "../include/common/Param.h"

#include <map>
#include <mutex>
#include <memory>
#include <vector>

#include <Windows.h>

#include "HookCommon.h"

#include "IHookObserver.h"

typedef std::string HookId;

struct HookIdInfo;

/*
 * 单例类，需要管理所有已安装hook的状态
 * !!!注意，这个类是注入的dll使用的，而不是给当前项目进程使用的
 */
class HookEngine
{
public:
    static HookEngine& GetInstance();

    bool StartHook(HookType type, const Param& param);

    bool StopHook(HookId id);

    // 注册观察者
    void RegisterObserver(IHookObserver* observer);

    // 注销观察者 注销观察者的目的是防止observer被析构后调用观察者导致出现无法预估的问题
    void UnregisterObserver(IHookObserver* observer);

private:
    // 计算hookid
    std::string GenerateHookId(HookType type, const Param& param, HookIdInfo* info);

    // 计算哈希，使用sha-256
    std::string CalculateHash(const std::string& strKey);

private:
    // 统一管理各个Hook的生命周期
    std::mutex m_mtxHookInfo;
    std::map<HookId, std::unique_ptr<HookIdInfo>> m_mapHookInfo;

    // 观察者 存在同时读写，需要加锁保证线程安全
    std::mutex m_mtxObserver;
    std::vector<IHookObserver*> m_vecObserver;

private:
    HookEngine();
    ~HookEngine();

    HookEngine(const HookEngine& other) = delete;
    HookEngine operator=(const HookEngine& other) = delete;
};

