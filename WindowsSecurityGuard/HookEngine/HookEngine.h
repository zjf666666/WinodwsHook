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
 * �����࣬��Ҫ���������Ѱ�װhook��״̬
 * !!!ע�⣬�������ע���dllʹ�õģ������Ǹ���ǰ��Ŀ����ʹ�õ�
 */
class HookEngine
{
public:
    static HookEngine& GetInstance();

    bool StartHook(HookType type, const Param& param);

    bool StopHook(HookId id);

    // ע��۲���
    void RegisterObserver(IHookObserver* observer);

    // ע���۲��� ע���۲��ߵ�Ŀ���Ƿ�ֹobserver����������ù۲��ߵ��³����޷�Ԥ��������
    void UnregisterObserver(IHookObserver* observer);

private:
    // ����hookid
    std::string GenerateHookId(HookType type, const Param& param, HookIdInfo* info);

    // �����ϣ��ʹ��sha-256
    std::string CalculateHash(const std::string& strKey);

private:
    // ͳһ�������Hook����������
    std::mutex m_mtxHookInfo;
    std::map<HookId, std::unique_ptr<HookIdInfo>> m_mapHookInfo;

    // �۲��� ����ͬʱ��д����Ҫ������֤�̰߳�ȫ
    std::mutex m_mtxObserver;
    std::vector<IHookObserver*> m_vecObserver;

private:
    HookEngine();
    ~HookEngine();

    HookEngine(const HookEngine& other) = delete;
    HookEngine operator=(const HookEngine& other) = delete;
};

