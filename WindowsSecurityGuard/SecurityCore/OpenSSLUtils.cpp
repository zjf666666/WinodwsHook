#include "pch.h"
#include "OpenSSLUtils.h"
#include <openssl/evp.h>

std::string OpenSSLUtils::Sha256(const std::string& str)
{
    // ����EVP������
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (nullptr == ctx)
    {
        return ""; // �ڴ����ʧ��
    }

    // ��ʼ��SHA256����
    if (1 != EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr))
    {
        EVP_MD_CTX_free(ctx);
        return ""; // ��ʼ��ʧ��
    }

    // ��������
    if (1 != EVP_DigestUpdate(ctx, str.c_str(), str.size()))
    {
        EVP_MD_CTX_free(ctx);
        return ""; // ���ݸ���ʧ��
    }

    // ������
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    if (1 != EVP_DigestFinal_ex(ctx, hash, &hash_len))
    {
        EVP_MD_CTX_free(ctx);
        return ""; // ���ռ���ʧ��
    }

    // �ͷ�������
    EVP_MD_CTX_free(ctx);

    // ת��Ϊʮ�������ַ���
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
