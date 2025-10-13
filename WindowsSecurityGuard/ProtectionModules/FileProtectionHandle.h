#pragma once

#include "IProtectionHandle.h"

class FileProtectionHandle : public IProtectionHandle
{
public:
    std::string Handle(CommandType type, Command cmd, const std::string& strJson) override;

private:
    std::string HandleCreateFileMonitor(CommandType type, Command cmd, const std::string& strJson);
};

