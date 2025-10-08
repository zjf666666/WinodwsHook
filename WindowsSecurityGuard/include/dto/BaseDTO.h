#pragma once

#include <string>

class BaseDTO
{
public:
    BaseDTO() = default;
    BaseDTO(const std::string& id, uint64_t time) : messageId(id), timestamp(time) {}
    virtual ~BaseDTO() = default;

    virtual std::string ToJson() const = 0;   // DTO转化为json函数
    virtual void FromJson(const std::string& jsonStr) = 0;  // json转化为DTO函数

protected:
    std::string messageId;      // 消息唯一标识 暂时不用
    uint64_t timestamp;         // 时间戳
};