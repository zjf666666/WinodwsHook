#pragma once

#include <string>

#include "../include/common/Param.h"

/* Hook �ӿ��࣬�������� Hook ʵ�ֵ�ͨ�ýӿ� */
class IHook
{
public:
    virtual ~IHook() = default; // �������ִ�C++���ϰ�� ��ͬ�� ~IHook() {}

    // ��ʼ������
    virtual bool Init(const Param& params) = 0;

    // ��װ Hook
    virtual bool Install() = 0;

    // ж�� Hook
    virtual bool Uninstall() = 0;

    // ��� Hook �Ƿ��Ѱ�װ
    virtual bool IsInstalled() const = 0;
};

