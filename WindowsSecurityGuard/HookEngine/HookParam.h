#pragma once

#include "IHookParam.h"

#include <string>

class InlineHookParam : public IHookParam
{
public:
    std::wstring targetModule; // hook�ĸ�ģ�� ��kernel32.dll
    std::string targetFunction; // hookģ����ĸ����� ��CreateFileW����
    void* hookFunction;  // �滻��HOOK�ĺ����ĺ��� ���û��Լ���д��MyCreateFileW
    bool bIs64Bit;  // �ܹ���Ϣ
};

class IATHookParam : public IHookParam
{
public:
    std::wstring targetModule;
    std::string targetFunction;
    void* pHookFunction;
    bool bIs64Bit;  // �ܹ���Ϣ ��Ϊע���ʱ���Ѿ���Ҫ�жϼܹ��ˣ������������ ��Ϊһ����ֵ֪���룬������hook���ж�
};


