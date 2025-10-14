#pragma once

#include <memory>
#include "NamedPipeServer.h"

class SecurityService
{
public:
    bool Initialize();
    bool Start();

private:
    std::unique_ptr<NamedPipeServer> m_pipeServer;
    std::shared_ptr<CommandRegistry> m_cmdReg;
    // 其他成员...
};

