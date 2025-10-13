#pragma once

#include "IDTO.h"
#include "GenerateCommonParam.h"

class RequestDTO : public IDTO
{
public:
    RequestDTO() = default;
    virtual ~RequestDTO() = default;

    virtual std::string ToJson() const = 0;   // DTO转化为json函数
    virtual void FromJson(const std::string& jsonStr) = 0;  // json转化为DTO函数
    void init()
    {
        messageId = GenerateCommonParam::GenerateMessageId();
        timestamp = GenerateCommonParam::GenerateTimeStamp();
        requestId = GenerateCommonParam::GenerateRequestId();
        timeOutMs = GenerateCommonParam::GenerateTimeOut();
    }

protected:
    std::string messageId;
    uint64_t timestamp;

    std::string requestId;
    uint32_t timeOutMs;
};