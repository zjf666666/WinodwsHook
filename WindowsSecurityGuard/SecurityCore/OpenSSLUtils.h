#pragma once

#include <string>
/*
 * 封装OpenSSL库相关操作
 */
class OpenSSLUtils
{
public:
    // 计算sha256
    static std::string Sha256(const std::string& str);
};

