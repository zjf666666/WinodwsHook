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
    //std::unique_ptr<MessageDispatcher> m_cmdRegistry;
    // 其他成员...
};

