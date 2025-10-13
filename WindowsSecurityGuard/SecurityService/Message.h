#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace WindowsSecurityGuard
{
    // 协议常量
    static constexpr uint32_t kMsgMagic = 0x47555357;        // 'WSUG' 魔数
    static constexpr uint32_t kProtocolVersion = 1;          // 协议版本
    static constexpr uint32_t kMaxMessageSize = 1024 * 1024; // 最大消息大小 1MB

    // 消息头结构体（定长）
    struct MessageHeader
    {
        uint32_t magic;         // 魔数，用于快速校验协议（固定为 kMsgMagic）
        uint32_t version;       // 协议版本，便于兼容（建议：1）
        uint32_t messageType;   // 消息类型（message 的枚举值）
        uint32_t commandType;   // 命令类型（CommandType 的枚举值）
        uint32_t cmd;           //命令字 （Command 的枚举值）
        uint32_t correlationId; // 关联 ID，用于请求-响应匹配（由客户端递增生成）
        uint32_t flags;         // 标志位，保留扩展（如压缩/加密/分片等）
        uint32_t payloadLength; // 负载长度（字节数），后续读取 payloadLength 字节的负载
        uint64_t timestamp;     // 时间戳（可选，用于调试和日志）
        uint32_t size;          // 消息体大小

        // 构造函数
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

    // 完整消息结构体
    struct Message
    {
        MessageHeader header;       // 协议头部
        std::vector<uint8_t> body;  // 负载数据（一般为 UTF-8 编码的 JSON 文本）

        // 构造函数
        Message() = default;

        Message(MessageType type, uint32_t correlationId = 0)
        {
            header.messageType = static_cast<uint32_t>(type);
            header.correlationId = correlationId;
        }

        // 请求响应用这个构造函数
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

        // 设置消息体
        void SetBody(const std::string& jsonData)
        {
            body.assign(jsonData.begin(), jsonData.end());
            header.size = static_cast<uint32_t>(body.size());
        }

        // 获取消息体为字符串
        std::string GetBodyAsString() const
        {
            if (body.empty())
            {
                return "";
            }
            return std::string(body.begin(), body.end());
        }

        // 获取消息类型
        MessageType GetType() const
        {
            return static_cast<MessageType>(header.messageType);
        }
    };

    // 协议工具类
    class ProtocolUtils
    {
    public:
        // 验证消息头
        static bool ValidateHeader(const MessageHeader& header)
        {
            return header.magic == kMsgMagic &&
                header.version == kProtocolVersion &&
                header.size <= kMaxMessageSize;
        }
    };
}