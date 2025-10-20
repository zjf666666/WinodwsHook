#include "pch.h"
#include "FileProtectionHandle.h"

#include "../include/dto/DTOFactory.h"
#include "../include/common/Error.h"
#include "../SecurityCore/ProcessUtils.h"
#include "../SecurityCore/ProcessInjectionManager.h"

std::string FileProtectionHandle::Handle(CommandType type, Command cmd, const std::string& strJson)
{
    switch (cmd)
    {
    case Command::FILE_ADD_MONITOR_CREATE:
        return HandleCreateFileMonitor(type, cmd, strJson);
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
    return "";
}

std::string FileProtectionHandle::HandleCreateFileMonitor(CommandType type, Command cmd, const std::string& strJson)
{
    IDTO* dto = DTOFactory::Create(MessageType::REQUEST, type, cmd);
    dto->FromJson(strJson); // 解析json
    std::wstring wstrDir = ProcessUtils::GetCurrentProcessDir();
    auto pid = dto->param.Get<int>("request_process_id");
    ErrorCode error = ErrorCode::SUCCESS;
    if (false == ProcessInjectionManager::GetInstance().InjectDll(*pid, wstrDir + std::wstring(L"IATHOOKDLL.dll")))
    {
        error = ErrorCode::INJECT_FAILED;
    }
    delete dto;

    // TODO: 需要requestid,messageid等信息
    dto = DTOFactory::Create(MessageType::RESPONSE, type, cmd);
    dto->param.Set<ErrorCode>("response_error_code", error);
    dto->param.Set<std::string>("response_error_message", ErrorManager::GetErrorMessage(error));

    return dto->ToJson();
}
