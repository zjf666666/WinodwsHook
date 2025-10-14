#pragma once

#include "RequestDTO.h"

class CreateFileMonitorDTO : public RequestDTO
{
public:
    CreateFileMonitorDTO();
    ~CreateFileMonitorDTO();

    std::string ToJson() const override;   // DTOת��Ϊjson����
    void FromJson(const std::string& jsonStr) override;  // jsonת��ΪDTO����
};