#pragma once

/*
 * 消息抽象接口
 * 解决handle入参强依赖于消息头的问题，
 * 例如cmd、cmdtype、messageid、requestid等参数需要在响应中包含
 * 这导致handle函数入参需要包含这些信息，一旦消息头又添加了新的必需字段
 * handle函数就需要修改，违背了开闭原则
 */

// 空类，仅用于隐藏业务信息细节
class IBussinessHeader
{

};

class IMessage
{
public:
    virtual ~IMessage() = default;
    virtual IBussinessHeader* GetBussinessHeader() const = 0;
};