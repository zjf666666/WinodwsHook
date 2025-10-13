#pragma once

#include <string>
#include "../include/common/Param.h"

class IDTO
{
public:
    IDTO() = default;
    virtual ~IDTO() = default;

    virtual void init() = 0; // 初始化函数，减少必要参数初始化逻辑，减少冗余代码
    virtual std::string ToJson() const = 0;   // DTO转化为json函数
    virtual void FromJson(const std::string& jsonStr) = 0;  // json转化为DTO函数

    // 为了解耦引入param，这样的处理会导致数据需要做两次转换
    // 1. 参数->param，完全是new操作，和栈上操作性能相差极大，且容易产生碎片
    // 2. param->json，这里是无法避免的，性能差距不大
    // 同样的解析时也会面临这个问题，这种设计方式是否合理待考虑，如果不考虑高频场景（几十万/s）是没有问题的
    Param param;
};