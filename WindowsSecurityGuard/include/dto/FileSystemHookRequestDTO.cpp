#include "pch.h"
#include "FileSystemHookRequestDTO.h"
#include "nlohmann/json.hpp"

FileSystemHookRequest::FileSystemHookRequest()
{

}

FileSystemHookRequest::FileSystemHookRequest(
    const std::string& messageId,
    uint64_t timestamp,
    const std::string& requestId,
    uint32_t timeout,
    int pid,
    const std::string& name
) : RequestDTO(messageId, timestamp, requestId, timeout), nPid(pid), strProcessName(name)
{

}

FileSystemHookRequest::~FileSystemHookRequest()
{

}

std::string FileSystemHookRequest::ToJson() const
{
    nlohmann::json json;
    json["messageId"]   = messageId;
    json["timestamp"]   = timestamp;
    json["requestId"]   = requestId;
    json["timeout"]     = timeoutMs;
    json["pid"]         = nPid;
    json["processName"] = strProcessName;
    return json.dump();
}

void FileSystemHookRequest::FromJson(const std::string& jsonStr)
{
    nlohmann::json json = nlohmann::json::parse(jsonStr);
    messageId = json["messageId"].get<std::string>();
    timestamp = json["timestamp"].get<uint64_t>();
    requestId = json["requestId"].get<std::string>();
    timeoutMs = json["timeout"].get<uint64_t>();
    nPid      = json["pid"].get<int>();
    strProcessName = json["processName"].get<std::string>();
}

