#include "pch.h"

#include "DTOFactory.h"
#include "FileRequestDTO.h"
#include "ResponseDTO.h"

IDTO* DTOFactory::Create(MessageType msgType, CommandType cmdType, Command cmd)
{
    IDTO* dto = nullptr;
    switch (msgType)
    {
    case MessageType::REQUEST:
        dto = CreateByType(cmdType, cmd);
        break;
    case MessageType::RESPONSE:
        dto = CreateResponseDTO(cmd);
        break;
    default:
        break;
    }

    if (!dto)
    {
        return nullptr;
    }

    dto->init(); // 工厂内执行dto初始化操作，避免遗漏导致缺少必要数据

    return dto;
}

IDTO* DTOFactory::CreateByType(CommandType cmdType, Command cmd)
{
    IDTO* dto = nullptr;
    switch (cmdType)
    {
    case CommandType::OPERATE_FILE:
        dto = CreateFileDTO(cmd);
        break;
    default:
        break;
    }

    if (!dto)
    {
        return nullptr;
    }

    dto->init(); // 工厂内执行dto初始化操作，避免遗漏导致缺少必要数据

    return dto;
}

IDTO* DTOFactory::CreateFileDTO(Command cmd)
{
    switch (cmd)
    {
    case Command::FILE_ADD_MONITOR_CREATE:
        return new CreateFileMonitorDTO();
        break;
    case Command::FILE_ADD_MONITOR_CHANGE:
        break;
    case Command::FILE_ADD_MONITOR_DELETE:
        break;
    case Command::FILE_DEL_MONITOR_CREATE:
        break;
    case Command::FILE_DEL_MONITOR_CHANGE:
        break;
    case Command::FILE_DEL_MONITOR_DELETE:
        break;
    case Command::FILE_QUERY_MONITOR_CREATE:
        break;
    case Command::FILE_QUERY_MONITOR_CHANGE:
        break;
    case Command::FILE_QUERY_MONITOR_DELETE:
        break;
    default:
        break;
    }
}

IDTO* DTOFactory::CreateResponseDTO(Command cmd)
{
    return new ResponseDTO();
}
