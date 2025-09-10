#pragma once
#include "IInjectionStrategy.h"

class CreateRemoteThreadStrategy : public IInjectionStrategy
{
public:
    virtual bool Inject(HANDLE hProcess, const std::wstring& dllPath) override;
    virtual std::wstring GetStrategyName() const override;
};

class QueueUserAPCStrategy : public IInjectionStrategy
{
public:
    virtual bool Inject(HANDLE hProcess, const std::wstring& dllPath) override;
    virtual std::wstring GetStrategyName() const override;
};


