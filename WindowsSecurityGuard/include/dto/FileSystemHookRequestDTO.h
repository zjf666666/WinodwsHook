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

    virtual std::string ToJson() const override;   // DTOת��Ϊjson����
    virtual void FromJson(const std::string& jsonStr) override;  // jsonת��ΪDTO����

    int nPid;   // ����id
    std::string strProcessName;  // ��������
    // TODO: ��ѡ�ֶ���չ
    std::string strInjectDllPath;  // ֧���Զ���ע��dll
    int nInjectType;   // ֧��ѡ��ע�뷽ʽ
    int nHookType;     // ֧��ѡ��HOOK��ʽ
};