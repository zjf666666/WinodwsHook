#pragma once

#include "RequestDTO.h"

class ConfigRequestDTO : public RequestDTO
{
public:
    std::string strKey;
    std::string strValue;

    // �Ȳ�ʵ��
    virtual std::string ToJson() const override
    {
        return "";
    }
    virtual void FromJson(const std::string& jsonStr) override
    {
    }  // jsonת��ΪDTO����
};