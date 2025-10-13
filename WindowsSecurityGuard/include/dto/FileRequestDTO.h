#pragma once

#include "RequestDTO.h"

class CreateFileMonitorDTO : public RequestDTO
{
public:
    CreateFileMonitorDTO();
    ~CreateFileMonitorDTO();

    virtual std::string ToJson() const override;   // DTOת��Ϊjson����
    virtual void FromJson(const std::string& jsonStr) override;  // jsonת��ΪDTO����
};