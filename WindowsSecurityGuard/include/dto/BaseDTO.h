#pragma once

#include <string>

class BaseDTO
{
public:
    BaseDTO() = default;
    BaseDTO(const std::string& id, uint64_t time) : messageId(id), timestamp(time) {}
    virtual ~BaseDTO() = default;

    virtual std::string ToJson() const = 0;   // DTOת��Ϊjson����
    virtual void FromJson(const std::string& jsonStr) = 0;  // jsonת��ΪDTO����

protected:
    std::string messageId;      // ��ϢΨһ��ʶ ��ʱ����
    uint64_t timestamp;         // ʱ���
};