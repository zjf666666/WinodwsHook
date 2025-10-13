#include "pch.h"
#include "ResponseDTO.h"
#include "nlohmann/json.hpp"

ResponseDTO::ResponseDTO()
{

}

ResponseDTO::~ResponseDTO()
{

}

void ResponseDTO::init()
{
    // ��Ӧ����Ҫ��ʼ����Ӧ��ֵ��Ҫ�������л�ȡ
}

std::string ResponseDTO::ToJson() const
{
    auto messageId = param.Get<uint32_t>("response_message_id");
    auto requestId = param.Get<uint32_t>("response_request_id");

    auto status = param.Get<bool>("response_status");
    auto errorCode = param.Get<uint32_t>("response_error_code");
    auto errorMessage = param.Get <std::string> ("response_error_message");

    if (nullptr == messageId ||
        nullptr == requestId ||
        nullptr == status ||
        nullptr == errorCode ||
        nullptr == errorMessage)
    {
        return "";
    }

    nlohmann::json json;
    json["messageId"]    = *messageId;
    json["requestId"]    = *requestId;
    json["status"]       = *status;
    json["errorCode"]    = *errorCode;
    json["errorMessage"] = *errorMessage;

    return json.dump();
}

void ResponseDTO::FromJson(const std::string& jsonStr)
{
    // Response����Ҫ������ֻ�з��ص�ʱ�����Ҫ
}

