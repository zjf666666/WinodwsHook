#pragma once

#include "BaseDTO.h"

class RequestDTO : public BaseDTO
{
public:
    RequestDTO() = default;
    RequestDTO(const std::string& messageId, uint64_t timestamp, const std::string& id, uint32_t time) :
        BaseDTO(messageId, timestamp), requestId(id), timeoutMs(time) {
    }
    virtual ~RequestDTO() = default;

    virtual std::string ToJson() const = 0;   // DTO转化为json函数
    virtual void FromJson(const std::string& jsonStr) = 0;  // json转化为DTO函数

protected:
    std::string requestId;      // 请求ID，用于响应关联
    uint32_t timeoutMs = 5000;  // 超时时间
};