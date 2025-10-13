#pragma once

#include "RequestDTO.h"

class CreateFileMonitorDTO : public RequestDTO
{
public:
    CreateFileMonitorDTO();
    ~CreateFileMonitorDTO();

    virtual std::string ToJson() const override;   // DTO转化为json函数
    virtual void FromJson(const std::string& jsonStr) override;  // json转化为DTO函数
};