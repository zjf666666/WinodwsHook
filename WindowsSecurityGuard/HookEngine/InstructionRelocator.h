#pragma once

#include <Windows.h>

#include "InstructionTypes.h"

/*
 * �ض���ûզ������ʱ�����ˣ����÷�װ��Zydisʵ��
 */

class InstructionRelocator
{
public:
    // �ض�λ����ָ��
    static BOOL RelocateInstruction(const InstructionInfo_study* instInfo, BYTE* newLocation, UINT* newLength, InstructionArchitecture arch);

private:
    static BOOL RelocateInstructionX86(const InstructionInfo_study* instInfo, BYTE* newLocation, UINT* newLength);
    
    static BOOL RelocateInstructionX64(const InstructionInfo_study* instInfo, BYTE* newLocation, UINT* newLength);

private:
    // �ض�λ�����תָ��
    static BOOL RelocateRelativeJump(const InstructionInfo_study* instInfo, BYTE* newLocation, UINT* newLength);

    // �ض�λ��Ե���ָ��
    static BOOL RelocateRelativeCall(const InstructionInfo_study* instInfo, BYTE* newLocation, UINT* newLength);

    // �ض�λ������תָ��
    static BOOL RelocateConditionalJump(const InstructionInfo_study* instInfo, BYTE* newLocation, UINT* newLength);

    // ����������תָ��
    static BOOL CreateAbsoluteJump(BYTE* targetAddress, BYTE* buffer, UINT* length);

    // ��ȡԭ����ָ��ָ���ַ
    static UINT_PTR GetOriginTargetAddress(const InstructionInfo_study* instInfo);
};

