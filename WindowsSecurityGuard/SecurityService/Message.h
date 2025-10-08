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

    // 消息类型枚举
    enum class MessageType : uint32_t
    {
        // 基础命令
        GetStatus = 100, // 获取服务状态
        ToggleProtection = 101, // 启停保护

        // 进程管理
        ListProcesses = 200, // 列出进程
        InjectHook = 300, // 对指定进程执行注入/安装 Hook

        // 事件订阅
        SubscribeEvents = 400, // 订阅事件推送
        UnsubscribeEvents = 401, // 取消订阅

        // 系统消息
        Heartbeat = 900, // 心跳保活
        HeartbeatResponse = 901, // 心跳响应

        // 错误处理
        Error = 999, // 错误响应
    };

    // 消息头结构体（定长）
    struct MessageHeader
    {
        uint32_t magic;         // 魔数，用于快速校验协议（固定为 kMsgMagic）
        uint32_t version;       // 协议版本，便于兼容（建议：1）
        uint32_t type;          // 消息类型（MessageType 的枚举值）
        uint32_t correlationId; // 关联 ID，用于请求-响应匹配（由客户端递增生成）
        uint32_t flags;         // 标志位，保留扩展（如压缩/加密/分片等）
        uint32_t payloadLength; // 负载长度（字节数），后续读取 payloadLength 字节的负载
        uint64_t timestamp;     // 时间戳（可选，用于调试和日志）
        uint32_t size; // 消息体长度

        // 构造函数
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

    // 完整消息结构体
    struct Message
    {
        MessageHeader header;       // 协议头部
        std::vector<uint8_t> body;  // 负载数据（一般为 UTF-8 编码的 JSON 文本）

        // 构造函数
        Message() = default;

        // 便利构造函数
        Message(MessageType type, uint32_t correlationId = 0)
        {
            header.type = static_cast<uint32_t>(type);
            header.correlationId = correlationId;
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
            return static_cast<MessageType>(header.type);
        }
    };

    // 协议工具类
    class ProtocolUtils
    {
    public:
        // 编码消息到二进制缓冲区（包含头部与负载）；返回是否成功
        static bool Encode(const Message& msg, std::vector<uint8_t>& outBuffer)
        {
            
        }

        // 从字节缓冲区解码为消息对象；返回是否成功
        static bool Decode(const uint8_t* data, size_t size, Message& outMsg);

        // 创建错误响应
        static Message MakeError(uint32_t correlationId, int errorCode, const std::string& errorMessage)
        {
            Message response(MessageType::Error, correlationId);

            // 创建错误JSON
            std::string errorJson = "{"
                "\"error_code\":" + std::to_string(errorCode) + ","
                "\"error_message\":\"" + errorMessage + "\""
                "}";

            response.SetBody(errorJson);
            return response;
        }

        // 创建成功响应
        static Message MakeResponse(MessageType type, uint32_t correlationId, const std::string& data = "")
        {
            Message response(type, correlationId);
            if (!data.empty())
            {
                response.SetBody(data);
            }
            return response;
        }

        // 验证消息头
        static bool ValidateHeader(const MessageHeader& header)
        {
            return header.magic == kMsgMagic &&
                header.version == kProtocolVersion &&
                header.size <= kMaxMessageSize;
        }
    };
}