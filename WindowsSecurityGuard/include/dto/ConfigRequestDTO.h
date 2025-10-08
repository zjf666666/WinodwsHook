#pragma once

#include "RequestDTO.h"

class ConfigRequestDTO : public RequestDTO
{
public:
    std::string strKey;
    std::string strValue;

    ConfigRequestDTO(
        const std::string& messageId, 
        uint64_t timestamp, 
        const std::string& requestId, 
        uint32_t timeout,
        const std::string& key,
        const std::string& val
    ) : RequestDTO(messageId, timestamp, requestId, timeout), strKey(key), strValue(val) {}
    ~ConfigRequestDTO() = default;

    // 先不实现
    virtual std::string ToJson() const override
    {
        return "";
    }
    virtual void FromJson(const std::string& jsonStr) override
    {
    }  // json转化为DTO函数
};