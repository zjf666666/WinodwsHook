#pragma once

#include "IHookParam.h"

#include <string>

class InlineHookParam : public IHookParam
{
public:
    std::wstring targetModule; // hook哪个模块 如kernel32.dll
    std::string targetFunction; // hook模块的哪个函数 如CreateFileW函数
    void* hookFunction;  // 替换被HOOK的函数的函数 如用户自己编写的MyCreateFileW
    bool bIs64Bit;  // 架构信息
};

class IATHookParam : public IHookParam
{
public:
    std::wstring targetModule;
    std::string targetFunction;
    void* pHookFunction;
    bool bIs64Bit;  // 架构信息 因为注入的时候已经需要判断架构了，所以这个参数 作为一个已知值传入，不再在hook内判断
};


