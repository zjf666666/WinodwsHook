#pragma once

#include "InstructionTypes.h"

/*
 * �ض���ӿ��࣬�ⲿ�ֽ�����ѧϰ�ü���
 * ����ʹ�û�ѡ�������ѷ�װ�õ��ض�����
 */
class IInstructionRelocator {
public:
    virtual ~IInstructionRelocator() {}

    // �ض�λ����ָ��
    virtual BOOL RelocateInstruction(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength) = 0;

    // �ض�λһ�δ���
    virtual BOOL RelocateCode(BYTE* sourceCode, UINT minLength, BYTE* targetBuffer, UINT* relocatedLength) = 0;

    // ������תָ�����λ������ԭλ�ã�
    virtual BOOL CreateJumpBack(BYTE* sourceLocation, BYTE* targetLocation, BYTE* buffer, UINT* length) = 0;
};

