#pragma once
#include <string>

#include <Windows.h>

// ����ģʽ����
class IInjectionStrategy
{
public:
    virtual ~IInjectionStrategy() {}
    virtual bool Inject(DWORD pid, const std::wstring& dllPath) = 0;
    virtual bool Eject(DWORD pid, const std::wstring& dllPath) = 0;
    virtual std::wstring GetStrategyName() const = 0;
};

