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

    virtual std::string ToJson() const = 0;   // DTOת��Ϊjson����
    virtual void FromJson(const std::string& jsonStr) = 0;  // jsonת��ΪDTO����

protected:
    std::string requestId;      // ����ID��������Ӧ����
    uint32_t timeoutMs = 5000;  // ��ʱʱ��
};