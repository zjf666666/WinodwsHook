#pragma once

// 站在业务角度定义所有的command，command是业务可感知的，需要确保业务可读性

typedef unsigned int       uint32_t;

// 消息模式，请求/响应/日志
enum class MessageType : uint32_t
{
    REQUEST,
    RESPONSE,
    EVENT
};

enum class CommandType : uint32_t
{
    // 请求类（回复和请求使用相同的命令字）
    OPERATE_FILE = 0x10000,     // 文件操作
    OPERATE_PROCESS = 0x20000,  // 进程操作
    OPERATE_NETWORK = 0x30000,  // 网络操作

    // 日志类
    EVENT_FILE = 0x100,     // 文件日志
    EVENT_PROCESS = 0x200,  // 进程日志
    EVENT_NETWORK = 0x300,  // 网络日志
};

enum class Command : uint32_t
{
    // 最多支持10种文件监控添加操作
    FILE_ADD_MONITOR_CREATE = 0x10000, // 增加创建文件监控
    FILE_ADD_MONITOR_CHANGE,           // 增加修改文件监控
    FILE_ADD_MONITOR_DELETE,           // 增加删除文件监控

    FILE_DEL_MONITOR_CREATE = 0x10010, // 增加创建文件监控
    FILE_DEL_MONITOR_CHANGE,           // 增加修改文件监控
    FILE_DEL_MONITOR_DELETE,           // 增加删除文件监控

    FILE_QUERY_MONITOR_CREATE = 0x10020, // 增加创建文件监控
    FILE_QUERY_MONITOR_CHANGE,           // 增加修改文件监控
    FILE_QUERY_MONITOR_DELETE,           // 增加删除文件监控
};

