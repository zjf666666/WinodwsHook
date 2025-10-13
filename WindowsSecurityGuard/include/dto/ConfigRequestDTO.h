#pragma once

#include "RequestDTO.h"

class ConfigRequestDTO : public RequestDTO
{
public:
    std::string strKey;
    std::string strValue;

    // 先不实现
    virtual std::string ToJson() const override
    {
        return "";
    }
    virtual void FromJson(const std::string& jsonStr) override
    {
    }  // json转化为DTO函数
};