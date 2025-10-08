#pragma once

#include "RequestDTO.h"

class FileSystemHookRequest : public RequestDTO
{
public:
    FileSystemHookRequest();
    FileSystemHookRequest(
        const std::string& messageId,
        uint64_t timestamp,
        const std::string& requestId,
        uint32_t timeout,
        int pid,
        const std::string& name
    );
    ~FileSystemHookRequest();

    virtual std::string ToJson() const override;   // DTO转化为json函数
    virtual void FromJson(const std::string& jsonStr) override;  // json转化为DTO函数

    int nPid;   // 进程id
    std::string strProcessName;  // 进程名称
    // TODO: 可选字段扩展
    std::string strInjectDllPath;  // 支持自定义注入dll
    int nInjectType;   // 支持选择注入方式
    int nHookType;     // 支持选择HOOK方式
};