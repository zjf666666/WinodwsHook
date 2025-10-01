#pragma once

#include <string>

#include "HookCommon.h"

// hook状态标志
enum class HookStatus
{
    // 核心状态：由HookEngine内部使用，用户不可修改数值或删除
    Installing = 1,    // 正在安装（预留，用于异步安装场景）
    Installed = 2,     // 安装成功
    InstallFailed = 3, // 安装失败
    Uninstalling = 4,  // 正在卸载（预留）
    Uninstalled = 5,   // 卸载成功
    UninstallFailed = 6, // 卸载失败

    // 扩展区：用户可在此处添加自定义状态（值从100开始，避免冲突）
    UserDefinedStart = 100,
    // 例如：UserHookTriggered = 101（用户自定义的Hook触发状态）
};

// Hook事件
struct HookEvent {
    // timestamp表示“时间”
    // 进程名称+进程pid+目标模块+目标函数+线程id构成“地点”
    // “人物”归属于业务，由dll进行赋值（user字段）
    // “事件”由dll记录HOOK函数调用内容作为事件（userData字段）
    // 什么人 在 什么地点 做了什么事

    // 1. HookEngine必须赋值的字段
    uint64_t timestamp;       // 事件时间戳（毫秒级，由HookEngine用系统时间生成）
    std::string targetFunc;   // 目标函数名称
    std::string targetModule; // 目标模块名称
    std::string ProcessName;  // 进程名称
    uint32_t processId;       // 目标进程ID
    uint32_t threadId;        // 触发事件的线程ID 用于判断哪个线程频繁触发了hook
    int errorCode;            // 错误码（0表示成功）
    std::string errorMsg;     // 错误描述（如"权限不足"）

    // 2. 由DLL或用户填充的字段（HookEngine仅负责传递，不解析）
    std::string user;         // 操作人（如Service的用户名，由DLL在注册时传入）
    void* userData;           // 自定义业务数据（DLL可存放指针，使用时自行解析）
};

class IHookObserver
{
public:
    virtual ~IHookObserver() = default;

    /**
     * @brief Hook 事件通知
     * @param [IN] event 事件类型
     *        [IN] hookInfo Hook 信息
     * @note 由 HookEngine 调用
     */
    virtual void OnHookEvent(HookStatus event, const HookEvent& hookInfo) = 0;
};