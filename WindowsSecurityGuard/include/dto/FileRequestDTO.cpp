#include "pch.h"
#include "FileRequestDTO.h"
#include "nlohmann/json.hpp"

CreateFileMonitorDTO::CreateFileMonitorDTO()
{

}

CreateFileMonitorDTO::~CreateFileMonitorDTO()
{

}

std::string CreateFileMonitorDTO::ToJson() const
{
    auto cmd = param.Get<uint32_t>("request_command");
    auto pid = param.Get<int>("request_process_id");
    auto processName = param.Get<std::string>("request_process_name");

    if (nullptr == cmd || 
        nullptr == pid || 
        nullptr == processName)
    {
        return "";
    }

    nlohmann::json json;
    json["messageId"] = messageId;
    json["timestamp"] = timestamp;
    json["requestId"] = requestId;
    json["timeout"] = timeOutMs;

    json["cmd"]         = *cmd;
    json["pid"]         = *pid;
    json["processName"] = *processName;

    auto injectDllPath = param.Get<std::string>("request_inject_dll_path");
    auto injectType = param.Get<std::string>("request_inject_type");
    auto hookType = param.Get<std::string>("request_hook_type");

    if (injectDllPath)
    {
        json["injectDllPath"] = *injectDllPath;
    }

    if (injectType)
    {
        json["injectType"] = *injectType;
    }

    if (hookType)
    {
        json["hookType"] = *hookType;
    }

    return json.dump();
}

void CreateFileMonitorDTO::FromJson(const std::string& jsonStr)
{
    nlohmann::json json = nlohmann::json::parse(jsonStr);
    messageId = json["messageId"].get<std::string>();
    timestamp = json["timestamp"].get<uint64_t>();
    requestId = json["requestId"].get<std::string>();
    timeOutMs = json["timeout"].get<uint64_t>();

    param.Set("request_command", json["cmd"].get<uint32_t>());  // ÃüÁî×Ö
    param.Set("request_process_id", json["pid"].get<int>());
    param.Set("request_process_name", json["processName"].get<std::string>());

    if (json.contains("injectDllPath"))
    {
        param.Set("request_inject_dll_path", json["injectDllPath"].get<std::string>());
    }

    if (json.contains("injectType"))
    {
        param.Set("request_inject_type", json["injectType"].get<std::string>());
    }

    if (json.contains("hookType"))
    {
        param.Set("request_hook_type", json["hookType"].get<std::string>());
    }
}

