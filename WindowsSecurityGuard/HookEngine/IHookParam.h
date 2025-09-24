#pragma once
#include "pch.h"

/*
 * 抽象数据类，用于统一IHOOK的install函数入参信息
 */
class IHookParam
{
public:
    virtual ~IHookParam() = default;
};