#pragma once

#include "RequestDTO.h"

class CreateFileMonitorDTO : public RequestDTO
{
public:
    CreateFileMonitorDTO();
    ~CreateFileMonitorDTO();

    std::string ToJson() const override;   // DTO转化为json函数
    void FromJson(const std::string& jsonStr) override;  // json转化为DTO函数
};