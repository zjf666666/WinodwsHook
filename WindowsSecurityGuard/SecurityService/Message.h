#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace WindowsSecurityGuard
{
    // Э�鳣��
    static constexpr uint32_t kMsgMagic = 0x47555357;        // 'WSUG' ħ��
    static constexpr uint32_t kProtocolVersion = 1;          // Э��汾
    static constexpr uint32_t kMaxMessageSize = 1024 * 1024; // �����Ϣ��С 1MB

    // ��Ϣ����ö��
    enum class MessageType : uint32_t
    {
        // ��������
        GetStatus = 100, // ��ȡ����״̬
        ToggleProtection = 101, // ��ͣ����

        // ���̹���
        ListProcesses = 200, // �г�����
        InjectHook = 300, // ��ָ������ִ��ע��/��װ Hook

        // �¼�����
        SubscribeEvents = 400, // �����¼�����
        UnsubscribeEvents = 401, // ȡ������

        // ϵͳ��Ϣ
        Heartbeat = 900, // ��������
        HeartbeatResponse = 901, // ������Ӧ

        // ������
        Error = 999, // ������Ӧ
    };

    // ��Ϣͷ�ṹ�壨������
    struct MessageHeader
    {
        uint32_t magic;         // ħ�������ڿ���У��Э�飨�̶�Ϊ kMsgMagic��
        uint32_t version;       // Э��汾�����ڼ��ݣ����飺1��
        uint32_t type;          // ��Ϣ���ͣ�MessageType ��ö��ֵ��
        uint32_t correlationId; // ���� ID����������-��Ӧƥ�䣨�ɿͻ��˵������ɣ�
        uint32_t flags;         // ��־λ��������չ����ѹ��/����/��Ƭ�ȣ�
        uint32_t payloadLength; // ���س��ȣ��ֽ�������������ȡ payloadLength �ֽڵĸ���
        uint64_t timestamp;     // ʱ�������ѡ�����ڵ��Ժ���־��
        uint32_t size; // ��Ϣ�峤��

        // ���캯��
        MessageHeader()
            : magic(kMsgMagic)
            , version(kProtocolVersion)
            , type(0)
            , correlationId(0)
            , flags(0)
            , payloadLength(0)
            , timestamp(0)
            , size(0)
        {
        }
    };

    // ������Ϣ�ṹ��
    struct Message
    {
        MessageHeader header;       // Э��ͷ��
        std::vector<uint8_t> body;  // �������ݣ�һ��Ϊ UTF-8 ����� JSON �ı���

        // ���캯��
        Message() = default;

        // �������캯��
        Message(MessageType type, uint32_t correlationId = 0)
        {
            header.type = static_cast<uint32_t>(type);
            header.correlationId = correlationId;
        }

        // ������Ϣ��
        void SetBody(const std::string& jsonData)
        {
            body.assign(jsonData.begin(), jsonData.end());
            header.size = static_cast<uint32_t>(body.size());
        }

        // ��ȡ��Ϣ��Ϊ�ַ���
        std::string GetBodyAsString() const
        {
            if (body.empty())
            {
                return "";
            }
            return std::string(body.begin(), body.end());
        }

        // ��ȡ��Ϣ����
        MessageType GetType() const
        {
            return static_cast<MessageType>(header.type);
        }
    };

    // Э�鹤����
    class ProtocolUtils
    {
    public:
        // ������Ϣ�������ƻ�����������ͷ���븺�أ��������Ƿ�ɹ�
        static bool Encode(const Message& msg, std::vector<uint8_t>& outBuffer)
        {
            
        }

        // ���ֽڻ���������Ϊ��Ϣ���󣻷����Ƿ�ɹ�
        static bool Decode(const uint8_t* data, size_t size, Message& outMsg);

        // ����������Ӧ
        static Message MakeError(uint32_t correlationId, int errorCode, const std::string& errorMessage)
        {
            Message response(MessageType::Error, correlationId);

            // ��������JSON
            std::string errorJson = "{"
                "\"error_code\":" + std::to_string(errorCode) + ","
                "\"error_message\":\"" + errorMessage + "\""
                "}";

            response.SetBody(errorJson);
            return response;
        }

        // �����ɹ���Ӧ
        static Message MakeResponse(MessageType type, uint32_t correlationId, const std::string& data = "")
        {
            Message response(type, correlationId);
            if (!data.empty())
            {
                response.SetBody(data);
            }
            return response;
        }

        // ��֤��Ϣͷ
        static bool ValidateHeader(const MessageHeader& header)
        {
            return header.magic == kMsgMagic &&
                header.version == kProtocolVersion &&
                header.size <= kMaxMessageSize;
        }
    };
}