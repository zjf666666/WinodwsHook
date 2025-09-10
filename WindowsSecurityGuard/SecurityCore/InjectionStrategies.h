#pragma once
#include "IInjectionStrategy.h"

class CreateRemoteThreadStrategy : public IInjectionStrategy
{
public:
    virtual bool Inject(DWORD pid, const std::wstring& dllPath) override;
    virtual bool Eject(DWORD pid, const std::wstring& dllPath) override;
    virtual std::wstring GetStrategyName() const override;
};

class QueueUserAPCStrategy : public IInjectionStrategy
{
public:
    virtual bool Inject(DWORD pid, const std::wstring& dllPath) override;
    virtual bool Eject(DWORD pid, const std::wstring& dllPath) override;
    virtual std::wstring GetStrategyName() const override;
};


