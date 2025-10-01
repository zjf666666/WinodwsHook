# UI 与 Service 命名管道通信实现文档

本文档描述在 WindowsSecurityGuard 项目中，SecurityGuardUI（UI 前端）与 SecurityService（服务后端）通过命名管道进行通信的整体设计、类划分、函数声明以及注释说明。实现目标是提供稳定、安全、可扩展的请求-响应与事件推送机制。

## 1. 通信模型与协议

- 通信通道：命名管道（`\\.\pipe\WindowsSecurityGuard`），全双工，消息型（`PIPE_TYPE_MESSAGE`），重叠 I/O（`FILE_FLAG_OVERLAPPED`）。
- 安全策略：通过 SDDL 配置管道的 DACL，允许系统与本地管理员、交互式用户访问。例如：`D:(A;;FA;;;SY)(A;;FA;;;BA)(A;;FA;;;IU)`。
- 帧结构：长度前缀协议，先读定长头部再读负载，避免黏包/半包。

```cpp
// 协议常量与类型
static constexpr uint32_t kMsgMagic = 0x47555357; // 'WSUG' 或 'WSGU' 均可，示例为 WSUG

enum class MessageType : uint32_t {
    GetStatus          = 100, // 获取服务状态
    ToggleProtection   = 101, // 启停保护
    ListProcesses      = 200, // 列出进程
    InjectHook         = 300, // 对指定进程执行注入/安装 Hook
    SubscribeEvents    = 400, // 订阅事件推送
    UnsubscribeEvents  = 401, // 取消订阅
    Heartbeat          = 900, // 心跳保活
    Error              = 999, // 错误响应
};

// 消息头（定长，长度前缀）
struct MessageHeader {
    uint32_t magic;         // 魔数，用于快速校验协议（固定为 kMsgMagic）
    uint32_t version;       // 协议版本，便于兼容（建议：1）
    uint32_t type;          // 消息类型（MessageType 的枚举值）
    uint32_t correlationId; // 关联 ID，用于请求-响应匹配（由客户端递增生成）
    uint32_t flags;         // 标志位，保留扩展（如压缩/加密/分片等）
    uint32_t payloadLength; // 负载长度（字节数），后续读取 payloadLength 字节的负载
};

// 消息体（示例）：负载建议使用 UTF-8 JSON，便于调试与扩展
struct Message {
    MessageHeader header;       // 协议头部
    std::vector<uint8_t> body;  // 负载数据（一般为 UTF-8 编码的 JSON 文本）
};
```

## 2. 服务端（SecurityService）类设计与函数声明

### 2.1 NamedPipeServer

职责：创建命名管道端点、接受客户端连接、为每个连接创建会话并管理生命周期。

```cpp
class NamedPipeServer {
public:
    // 初始化管道服务器，设置管道名称与安全描述（SDDL），不启动接受循环
    bool Initialize(const std::wstring& pipeName, const std::wstring& sddl);

    // 启动服务器接受循环（创建管道实例并异步等待连接），返回是否启动成功
    bool Start();

    // 停止服务器，通知所有会话关闭并释放资源（阻塞直到完全停止）
    void Stop();

    // 设置消息分发器（在接受到消息后进行路由处理）
    void SetDispatcher(std::shared_ptr<class MessageDispatcher> dispatcher);

    // 获取当前活动会话数（用于监控与诊断）
    size_t GetActiveSessionCount() const;

private:
    // 创建单个管道实例（调用 CreateNamedPipe），设置重叠 I/O 等属性
    HANDLE CreatePipeInstance();

    // 接受循环线程函数：等待客户端连接（ConnectNamedPipe），创建 ClientSession 并交给线程池/后台线程管理
    void AcceptLoop();

    // 根据 SDDL 构造 SECURITY_ATTRIBUTES，用于 CreateNamedPipe 安全设置
    bool BuildSecurityAttributes(const std::wstring& sddl, SECURITY_ATTRIBUTES& sa, std::vector<uint8_t>& securityDescriptorBuff);
};
```

### 2.2 ClientSession

职责：代表单个客户端连接，会话内负责收发消息、心跳维护、事件推送。

```cpp
class ClientSession : public std::enable_shared_from_this<ClientSession> {
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
```

### 2.3 MessageDispatcher

职责：统一路由处理请求，管理命令处理器注册，封装错误处理与日志记录。

```cpp
class MessageDispatcher {
public:
    using HandlerPtr = std::shared_ptr<class ICommandHandler>;

    // 注册处理器，将消息类型与处理器实例关联（重复注册将覆盖）
    void RegisterHandler(MessageType type, HandlerPtr handler);

    // 分发请求，返回响应消息（如出现异常，返回 Error 类型响应）
    Message Dispatch(const Message& request);

private:
    // 根据类型查找处理器，若不存在则返回错误
    HandlerPtr FindHandler(MessageType type) const;
};
```

### 2.4 ICommandHandler 及具体处理器

职责：定义统一的命令处理接口；每种消息类型实现一个处理器，便于扩展与测试。

```cpp
class ICommandHandler {
public:
    virtual ~ICommandHandler() = default;

    // 处理请求并生成响应；
    // 返回 true 表示处理成功（响应为正常类型），false 表示处理失败（建议由 Dispatcher 封装为 Error 响应）
    virtual bool Handle(const Message& request, Message& response) = 0;
};

// 以下为常用命令的处理器声明示例：
class GetStatusHandler : public ICommandHandler {
public:
    bool Handle(const Message& request, Message& response) override;
};

class ToggleProtectionHandler : public ICommandHandler {
public:
    // 根据请求中的 enable 字段启停保护模块
    bool Handle(const Message& request, Message& response) override;
};

class ListProcessesHandler : public ICommandHandler {
public:
    // 返回进程列表（结合 SecurityCore::ProcessUtils）
    bool Handle(const Message& request, Message& response) override;
};

class InjectHookHandler : public ICommandHandler {
public:
    // 对指定进程执行注入与 Hook 安装（结合 ProcessInjectionManager/HookEngine）
    bool Handle(const Message& request, Message& response) override;
};

class SubscribeEventsHandler : public ICommandHandler {
public:
    // 为当前会话登记事件订阅，后续异步推送事件
    bool Handle(const Message& request, Message& response) override;
};

class UnsubscribeEventsHandler : public ICommandHandler {
public:
    // 取消事件订阅，释放相关资源
    bool Handle(const Message& request, Message& response) override;
};

class HeartbeatHandler : public ICommandHandler {
public:
    // 心跳处理：更新时间并返回心跳响应
    bool Handle(const Message& request, Message& response) override;
};
```

### 2.5 Serializer / ProtocolUtils

职责：编码与解码消息（头部与负载），进行字段校验、版本兼容与错误封装。

```cpp
class ProtocolUtils {
public:
    // 编码消息到二进制缓冲区（包含头部与负载）；返回是否成功
    static bool Encode(const Message& msg, std::vector<uint8_t>& outBuffer);

    // 从字节缓冲区解码为消息对象；返回是否成功
    static bool Decode(const uint8_t* data, size_t size, Message& outMsg);

    // 构造标准错误响应（填充 Error 类型与错误码/信息）
    static Message MakeError(uint32_t correlationId, int errorCode, const std::string& errorMessage);

    // 简化构造响应头部（复制请求中的关联 ID 与版本等）
    static MessageHeader MakeResponseHeader(const MessageHeader& requestHeader, MessageType type, uint32_t payloadLength);
};
```

### 2.6 EventPublisher（可选）

职责：集中化事件推送管理，维护订阅关系，向对应会话推送。

```cpp
class EventPublisher {
public:
    // 订阅：登记会话与订阅过滤条件（如事件类型、等级等）
    bool Subscribe(std::weak_ptr<ClientSession> session, const std::string& filterJson);

    // 取消订阅：移除会话关联
    void Unsubscribe(std::weak_ptr<ClientSession> session);

    // 推送事件：遍历订阅者，发送事件消息（异步/队列化）
    void Publish(const Message& eventMsg);
};
```

## 3. 客户端（SecurityGuardUI）类设计与函数声明

### 3.1 NamedPipeClient

职责：封装管道连接、请求-响应与事件循环，提供超时与重连机制。

```cpp
class NamedPipeClient {
public:
    using EventCallback = std::function<void(const Message&)>;

    // 连接管道；支持 WaitNamedPipe 等待；timeoutMs 为连接超时，返回是否成功
    bool Connect(const std::wstring& pipeName, DWORD timeoutMs);

    // 断开连接并释放资源
    void Disconnect();

    // 发送请求并等待响应（匹配 correlationId）；返回是否成功
    bool SendRequest(const Message& request, Message& outResponse, DWORD timeoutMs);

    // 启动事件循环线程，接收服务端推送事件并回调到 UI（线程安全）
    bool StartEventLoop(EventCallback cb);

    // 停止事件循环线程
    void StopEventLoop();

    // 心跳保活（可定时调用）
    bool SendHeartbeat(DWORD timeoutMs);

private:
    // 读一条完整消息（长度前缀），返回是否成功
    bool ReadMessage(Message& outMsg, DWORD timeoutMs);

    // 写一条完整消息（长度前缀），返回是否成功
    bool WriteMessage(const Message& msg, DWORD timeoutMs);

    // 事件循环函数：持续读取事件消息并触发回调
    void EventLoop(EventCallback cb);
};
```

### 3.2 ServiceApi（UI 业务接口）

职责：面向 UI 封装业务接口，屏蔽协议细节，提供易用方法。

```cpp
class ServiceApi {
public:
    explicit ServiceApi(std::shared_ptr<NamedPipeClient> client);

    // 初始化连接（内部调用 NamedPipeClient::Connect），返回是否成功
    bool Initialize(const std::wstring& pipeName, DWORD timeoutMs);

    // 获取服务状态（例如：当前保护开关、版本信息、活动会话数）
    bool GetStatus(/* out */ std::string& statusJson);

    // 启停保护（enable 为 true/false）
    bool ToggleProtection(bool enable);

    // 获取进程列表（返回 JSON 文本或结构化数据）
    bool ListProcesses(/* out */ std::string& processesJson);

    // 对指定进程 ID 执行注入并安装 Hook
    bool InjectHook(uint32_t processId);

    // 订阅事件：传入过滤条件（JSON），并提供回调；内部启动事件循环
    bool SubscribeEvents(const std::string& filterJson, NamedPipeClient::EventCallback cb);

    // 取消订阅事件：停止事件循环
    void UnsubscribeEvents();
};
```

## 4. UI 集成建议（MFC）

- 在 `SecurityGuardUI` 主对话框初始化（`OnInitDialog`）中创建 `ServiceApi`，调用 `Initialize` 进行连接。
- 在按钮事件中调用 `ServiceApi` 对应方法：如“刷新状态”、“启停保护”、“注入 Hook”。
- 事件推送通过 `ServiceApi::SubscribeEvents` 注册回调，在回调中使用 `PostMessage` 或者将数据放入线程安全队列，交由 UI 线程更新界面。
- 所有网络/管道操作置于后台线程，避免阻塞 UI 线程。

## 5. 错误处理与健壮性

- 超时与重试：连接、读写设置合理超时，出现超时进行有限重试（指数退避）；多次失败后反馈到 UI。
- 断线重连：事件循环检测到断开后尝试重连（可提示用户）。
- 输入校验：服务端对负载进行类型与尺寸检查，限制最大 payload 长度，避免资源耗尽。
- 心跳保活：UI 端定期发送 Heartbeat，服务端记录并清理长时间无心跳的会话。

## 6. 日志与审计

- 关键操作均写入 `Logger`：连接/断开、请求类型与耗时、错误码、事件推送统计等。
- 便于定位问题与性能优化，支持根据 `correlationId` 追踪一次请求的全链路。

## 7. 测试建议

- 单元测试：对 `ProtocolUtils::Encode/Decode/MakeError` 进行校验；构造边界用例（空负载/超长负载/错误版本）。
- 集成测试：启动服务端模拟处理器，客户端发起 `GetStatus/ToggleProtection/SubscribeEvents` 并验证响应与事件推送。
- 回归测试：并发会话、断线重连、权限受限连接、异常负载等场景。

---

以上为命名管道通信的类设计与函数声明，后续可在 `WindowsSecurityGuard\SecurityService` 与 `WindowsSecurityGuard\SecurityGuardUI` 子项目中按该接口进行具体实现与集成。建议将协议常量与 `MessageHeader/Message` 放入可复用的头文件，以确保 UI 与 Service 对协议的一致性。