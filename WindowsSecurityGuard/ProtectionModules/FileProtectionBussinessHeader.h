#pragma once

#include <string>
#include "../include/common/IMessage.h"

struct FileProtectionBussinessHeader : public IBussinessHeader
{
    std::string strMessageId;
    std::string strRequestId;
    
};

