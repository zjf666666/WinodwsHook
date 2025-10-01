#pragma once

#include <Memory>

#include <Windows.h>

struct Message;

class ClientSession : public std::enable_shared_from_this<ClientSession>
{
public:
    // 构造函数，传入已连接的管道句柄与分发器；可选传入日志器等辅助对象
    ClientSession(HANDLE pipe, std::shared_ptr<class MessageDispatcher> dispatcher);

    // 启动会话处理线程（或将会话提交到线程池），开始读写循环
    bool Start();

    // 关闭会话，停止读写并释放资源（线程安全）
    void Close();

    // 发送事件（服务端异步推送）：将事件消息写入管道；返回是否发送成功
    bool SendEvent(const Message& eventMsg);

    // 获取最后一次心跳时间戳，便于超时清理
    uint64_t GetLastHeartbeatTs() const;

private:
    // 会话主循环：读取消息头与负载，解析后交由分发器处理，并写回响应
    void RunLoop();

    // 读取一条完整消息（处理半包/黏包，返回是否成功）
    bool ReadMessage(Message& outMsg, DWORD timeoutMs);

    // 写入一条完整消息（长度前缀写入，返回是否成功）
    bool WriteMessage(const Message& msg, DWORD timeoutMs);

    // 心跳处理：更新心跳时间，必要时回复心跳响应
    void HandleHeartbeat(const Message& req);
};

