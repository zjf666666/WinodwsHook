#pragma once
#include "IHookId.h"

class InlineHookId : public IHookId
{
public:
    std::string GenerateKey(const Param& param, HookIdInfo* info) override;
};

class IATHookId : public IHookId
{
public:
    std::string GenerateKey(const Param& param, HookIdInfo* info) override;
};

