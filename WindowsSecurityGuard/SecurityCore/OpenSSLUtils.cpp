#include "pch.h"
#include "OpenSSLUtils.h"
#include <openssl/evp.h>

std::string OpenSSLUtils::Sha256(const std::string& str)
{
    // 创建EVP上下文
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (nullptr == ctx)
    {
        return ""; // 内存分配失败
    }

    // 初始化SHA256计算
    if (1 != EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr))
    {
        EVP_MD_CTX_free(ctx);
        return ""; // 初始化失败
    }

    // 输入数据
    if (1 != EVP_DigestUpdate(ctx, str.c_str(), str.size()))
    {
        EVP_MD_CTX_free(ctx);
        return ""; // 数据更新失败
    }

    // 计算结果
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    if (1 != EVP_DigestFinal_ex(ctx, hash, &hash_len))
    {
        EVP_MD_CTX_free(ctx);
        return ""; // 最终计算失败
    }

    // 释放上下文
    EVP_MD_CTX_free(ctx);

    // 转换为十六进制字符串
    std::string result;
    result.reserve(hash_len * 2);
    for (unsigned int i = 0; i < hash_len; ++i)
    {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", hash[i]);
        result += buf;
    }

    return result;
}
