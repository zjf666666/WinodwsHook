#pragma once

#include <string>
#include "../include/common/Param.h"

class IDTO
{
public:
    IDTO() = default;
    virtual ~IDTO() = default;

    virtual void init() = 0; // ��ʼ�����������ٱ�Ҫ������ʼ���߼��������������
    virtual std::string ToJson() const = 0;   // DTOת��Ϊjson����
    virtual void FromJson(const std::string& jsonStr) = 0;  // jsonת��ΪDTO����

    // Ϊ�˽�������param�������Ĵ���ᵼ��������Ҫ������ת��
    // 1. ����->param����ȫ��new��������ջ�ϲ����������������ײ�����Ƭ
    // 2. param->json���������޷�����ģ����ܲ�಻��
    // ͬ���Ľ���ʱҲ������������⣬������Ʒ�ʽ�Ƿ��������ǣ���������Ǹ�Ƶ��������ʮ��/s����û�������
    Param param;
};