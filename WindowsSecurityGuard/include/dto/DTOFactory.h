#pragma once

#include "IDTO.h"
#include "../include/common/Command.h"

class DTOFactory
{
public:
    static IDTO* Create(MessageType msgType, CommandType cmdType, Command cmd);

private:
    // 第二层 暂时只有请求调用
    static IDTO* CreateByType(CommandType cmdType, Command cmd);

private:
    // 第三层
    static IDTO* CreateFileDTO(Command cmd);

    static IDTO* CreateResponseDTO(Command cmd);
};