#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>

#include <Windows.h>

class ClientSession;
class CommandRegistry;

class NamedPipeServer
{
public:
    NamedPipeServer();

    ~NamedPipeServer();

    // 初始化管道服务器，设置管道名称与安全描述（SDDL），不启动接受循环
    bool Initialize(const std::wstring& pipeName, const std::wstring& sddl);

    // 启动服务器接受循环（创建管道实例并异步等待连接），返回是否启动成功
    bool Start();

    // 停止服务器，通知所有会话关闭并释放资源（阻塞直到完全停止）
    void Stop();

    // 设置消息分发器（在接受到消息后进行路由处理）
    void SetCommandRegistry(std::shared_ptr<CommandRegistry> cmdReg);

    // 获取当前活动会话数（用于监控与诊断）
    size_t GetActiveSessionCount() const;

private:
    // 生成安全属性
    bool GenerateSecurityAttributes();

    // 接受循环线程函数：等待客户端连接（ConnectNamedPipe），创建 ClientSession 并交给线程池/后台线程管理
    void AcceptLoop();

    // 创建命名管道实例
    bool CreatePipeInstance();

    // 等待客户端连接
    int WaitClientConnect(HANDLE hHandle);

    // 创建客户端会话信息
    void CreateClientSession(HANDLE hHandle);

    // 清理客户端会话
    void ClearClientSession();

private:
    std::atomic<bool> m_bIsInit;      // 是否已初始化
    std::atomic<bool> m_bIsRunning;   // 是否正在运行
    std::wstring m_wstrPipName;       // 管道名称
    std::wstring m_wstrSDDL;          // sddl描述符
    std::thread m_thdAcceptLoop;      // 接收循环线程

    std::mutex m_mtxClientSession;
    std::vector<std::weak_ptr<ClientSession>> m_vecSession;  // 活跃会话列表

    std::shared_ptr<CommandRegistry> m_cmdRegistry;  // 消息分发器

    SECURITY_ATTRIBUTES m_sa;
};

