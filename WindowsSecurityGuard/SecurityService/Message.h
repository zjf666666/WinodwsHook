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

    // ��Ϣͷ�ṹ�壨������
    struct MessageHeader
    {
        uint32_t magic;         // ħ�������ڿ���У��Э�飨�̶�Ϊ kMsgMagic��
        uint32_t version;       // Э��汾�����ڼ��ݣ����飺1��
        uint32_t messageType;   // ��Ϣ���ͣ�message ��ö��ֵ��
        uint32_t commandType;   // �������ͣ�CommandType ��ö��ֵ��
        uint32_t cmd;           //������ ��Command ��ö��ֵ��
        uint32_t correlationId; // ���� ID����������-��Ӧƥ�䣨�ɿͻ��˵������ɣ�
        uint32_t flags;         // ��־λ��������չ����ѹ��/����/��Ƭ�ȣ�
        uint32_t payloadLength; // ���س��ȣ��ֽ�������������ȡ payloadLength �ֽڵĸ���
        uint64_t timestamp;     // ʱ�������ѡ�����ڵ��Ժ���־��
        uint32_t size;          // ��Ϣ���С

        // ���캯��
        MessageHeader()
            : magic(kMsgMagic)
            , version(kProtocolVersion)
            , messageType(0)
            , commandType(0)
            , cmd(0)
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

        Message(MessageType type, uint32_t correlationId = 0)
        {
            header.messageType = static_cast<uint32_t>(type);
            header.correlationId = correlationId;
        }

        // ������Ӧ��������캯��
        Message(
            MessageType msgType, 
            CommandType cmdType,
            Command command,
            uint32_t corId = 0
            )
        {
            header.messageType = static_cast<uint32_t>(msgType);
            header.commandType = static_cast<uint32_t>(cmdType);
            header.cmd = static_cast<uint32_t>(command);
            header.correlationId = static_cast<uint32_t>(corId);
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
            return static_cast<MessageType>(header.messageType);
        }
    };

    // Э�鹤����
    class ProtocolUtils
    {
    public:
        // ��֤��Ϣͷ
        static bool ValidateHeader(const MessageHeader& header)
        {
            return header.magic == kMsgMagic &&
                header.version == kProtocolVersion &&
                header.size <= kMaxMessageSize;
        }
    };
}