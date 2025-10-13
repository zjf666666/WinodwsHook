#pragma once
#include "pch.h"

#include <string>
#include <memory>

#include "HookCommon.h"
#include "../include/common/Param.h"
#include "IHook.h"

struct HookIdInfo
{
    HookType type;                  // HOOK����
    std::unique_ptr<IHook> hook;    // hookʵ��
    std::wstring wstrTargetModule;  // Ŀ��ģ������
    std::string strTargetFunc;      // Ŀ�꺯������
    std::string strArch;            // �ܹ�
};

/* hookid���ɳ����� */
class IHookId
{
public:
    virtual ~IHookId() = default;
    virtual std::string GenerateKey(const Param& param, HookIdInfo* info) = 0;

protected:
    std::string ReplaceSpecChar(const std::string& str);
};