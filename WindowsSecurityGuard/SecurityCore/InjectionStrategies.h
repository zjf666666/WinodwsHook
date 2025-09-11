#pragma once
#include "IInjectionStrategy.h"

class CreateRemoteThreadStrategy : public IInjectionStrategy
{
public:
    bool Inject(DWORD pid, const std::wstring& dllPath) override;
    bool Eject(DWORD pid, const std::wstring& dllPath) override;
    std::wstring GetStrategyName() const override;
};

class QueueUserAPCStrategy : public IInjectionStrategy
{
public:
    bool Inject(DWORD pid, const std::wstring& dllPath) override;
    bool Eject(DWORD pid, const std::wstring& dllPath) override;
    std::wstring GetStrategyName() const override;
};


