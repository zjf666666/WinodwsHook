#pragma once

#include <string>
#include <atomic>

#include <chrono>
#include <cstdint>

#define REQUEST_TIME_OUT   5000  // 请求5s超时时间

class GenerateCommonParam
{
public:
    // 生成消息id  TODO: messageid待实现
    static std::string GenerateMessageId()
    {
        return "";
    }

    // 生成请求id
    static std::string GenerateRequestId()
    {
        static std::atomic<unsigned long long> m_requestSeq = 0;
        return "req_" + std::to_string(++m_requestSeq);
    }

    // 生成时间戳
    static uint64_t GenerateTimeStamp()
    {
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        return milliseconds.count();
    }

    // 生成超时时间
    static uint32_t GenerateTimeOut()
    {
        return 5000;
    }
};