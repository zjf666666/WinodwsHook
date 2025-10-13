#pragma once

#include "IDTO.h"
#include "../include/common/Command.h"

class DTOFactory
{
public:
    static IDTO* Create(MessageType msgType, CommandType cmdType, Command cmd);

private:
    // �ڶ��� ��ʱֻ���������
    static IDTO* CreateByType(CommandType cmdType, Command cmd);

private:
    // ������
    static IDTO* CreateFileDTO(Command cmd);

    static IDTO* CreateResponseDTO(Command cmd);
};