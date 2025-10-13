#pragma once

#include <string>
#include <atomic>

#include <chrono>
#include <cstdint>

#define REQUEST_TIME_OUT   5000  // ����5s��ʱʱ��

class GenerateCommonParam
{
public:
    // ������Ϣid  TODO: messageid��ʵ��
    static std::string GenerateMessageId()
    {
        return "";
    }

    // ��������id
    static std::string GenerateRequestId()
    {
        static std::atomic<unsigned long long> m_requestSeq = 0;
        return "req_" + std::to_string(++m_requestSeq);
    }

    // ����ʱ���
    static uint64_t GenerateTimeStamp()
    {
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        return milliseconds.count();
    }

    // ���ɳ�ʱʱ��
    static uint32_t GenerateTimeOut()
    {
        return 5000;
    }
};