#include "pch.h"
#include "InstructionRelocator.h"
#include "../SecurityCore/Logger.h"

BOOL InstructionRelocator::RelocateInstruction(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength, InstructionArchitecture arch)
{
    BOOL bRes = FALSE;
    switch (arch)
    {
    case ARCH_X86:
        bRes = RelocateInstructionX86(instInfo, newLocation, newLength);
        break;
    case ARCH_X64:
        bRes = RelocateInstructionX64(instInfo, newLocation, newLength);
        break;
    default:
        break;
    }
    return bRes;
}

BOOL InstructionRelocator::RelocateInstructionX86(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength)
{
    if (nullptr == instInfo || nullptr == newLocation || nullptr == newLength)
    {
        Logger::GetInstance().Error(L"RelocateInstruction param has nullptr!");
        return FALSE;
    }

    BOOL bRes = FALSE;
    // ����ָ������ѡ��ͬ���ض�λ����
    switch (instInfo->type)
    {
    case InstructionType::INST_JMP_NEAR:
    case InstructionType::INST_JMP_SHORT:
        bRes = RelocateRelativeJump(instInfo, newLocation, newLength);
        break;
    case InstructionType::INST_CALL_NEAR:
        bRes = RelocateRelativeCall(instInfo, newLocation, newLength);
        break;
    case InstructionType::INST_COND_JMP:
        bRes = RelocateConditionalJump(instInfo, newLocation, newLength);
        break;
    case InstructionType::INST_NORMAL:
        // ��ָͨ��ֱ�Ӹ���
        memcpy(newLocation, instInfo->address, instInfo->length);
        *newLength = instInfo->length;
        bRes = TRUE;
        break;
    default:
        // ��֧�ֵ�ָ������
        break;
    }
    return bRes;
}

BOOL InstructionRelocator::RelocateRelativeJump(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength)
{
    if (nullptr == instInfo || nullptr == newLocation || nullptr == newLength)
    {
        Logger::GetInstance().Error(L"RelocateRelativeJump param has nullptr!");
        return FALSE;
    }

    // ��ȡԭʼ��תĿ���ַ
    UINT_PTR pOriginTargetAddr = GetOriginTargetAddress(instInfo);
    if (0 == pOriginTargetAddr)
    {
        Logger::GetInstance().Error(L"Failed to get origin target address!");
        return FALSE;
    }

    // �����µ�ƫ����������λ�õ�ԭʼĿ���ƫ�ƣ�
    INT_PTR pNewOffset = pOriginTargetAddr - ((UINT_PTR)newLocation + instInfo->length);

    // ���ƫ�����Ƿ��ڷ�Χ��
    if (InstructionType::INST_JMP_SHORT == instInfo->type)
    {
        // ����ת��Χ��� (-128 ~ 127)
        if (pNewOffset < -128 || pNewOffset > 127)
        {
            // ����ת��Χ���㣬��Ҫת��Ϊ����ת
            newLocation[0] = 0xE9; // ����ת������
            *(INT32*)(newLocation + 1) = (INT32)pNewOffset;
            *newLength = 5; // ����תָ���Ϊ5�ֽ�
        }
        else
        {
            // ���ֶ���ת
            newLocation[0] = 0xEB; // ����ת������
            newLocation[1] = (BYTE)pNewOffset;
            *newLength = 2; // ����תָ���Ϊ2�ֽ�
        }
    }
    else // INST_JMP_NEAR
    {
        // ����ת��Χ��飨32λ�з���������Χ��
        if (pNewOffset < INT_MIN || pNewOffset > INT_MAX)
        {
            Logger::GetInstance().Error(L"New offset out of range for near jump!");
            return FALSE;
        }

        // ��������תָ��
        newLocation[0] = 0xE9; // ����ת������
        *(INT32*)(newLocation + 1) = (INT32)pNewOffset;
        *newLength = 5; // ����תָ���Ϊ5�ֽ�
    }

    return TRUE;
}

BOOL InstructionRelocator::RelocateRelativeCall(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength)
{
    if (nullptr == instInfo || nullptr == newLocation || nullptr == newLength)
    {
        Logger::GetInstance().Error(L"RelocateRelativeCall param has nullptr!");
        return FALSE;
    }

    // ��ȡԭʼ����Ŀ���ַ
    UINT_PTR pOriginTargetAddr = GetOriginTargetAddress(instInfo);
    if (0 == pOriginTargetAddr)
    {
        Logger::GetInstance().Error(L"Failed to get origin target address!");
        return FALSE;
    }

    // �����µ�ƫ����������λ�õ�ԭʼĿ���ƫ�ƣ�
    INT_PTR pNewOffset = pOriginTargetAddr - ((UINT_PTR)newLocation + 5); // CALLָ��ȹ̶�Ϊ5�ֽ�

    // ���ƫ�����Ƿ��ڷ�Χ�ڣ�32λ�з���������Χ��
    if (pNewOffset < INT_MIN || pNewOffset > INT_MAX)
    {
        Logger::GetInstance().Error(L"New offset out of range for call instruction!");
        return FALSE;
    }

    // ������Ե���ָ��
    newLocation[0] = 0xE8; // CALL������
    *(INT32*)(newLocation + 1) = (INT32)pNewOffset;
    *newLength = 5; // CALLָ���Ϊ5�ֽ�

    return TRUE;
}

BOOL InstructionRelocator::RelocateConditionalJump(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength)
{
    if (nullptr == instInfo || nullptr == newLocation || nullptr == newLength)
    {
        Logger::GetInstance().Error(L"RelocateConditionalJump param has nullptr!");
        return FALSE;
    }

    // ��ȡԭʼ��תĿ���ַ
    UINT_PTR pOriginTargetAddr = GetOriginTargetAddress(instInfo);
    if (0 == pOriginTargetAddr)
    {
        Logger::GetInstance().Error(L"Failed to get origin target address!");
        return FALSE;
    }

    // �����µ�ƫ����������λ�õ�ԭʼĿ���ƫ�ƣ�
    INT_PTR pNewOffset = pOriginTargetAddr - ((UINT_PTR)newLocation + 2); // ��������תָ���Ϊ2�ֽ�

    // ���ƫ�����Ƿ��ڷ�Χ��
    if (pNewOffset >= -128 && pNewOffset <= 127)
    {
        // ƫ�����ڶ���ת��Χ�ڣ�ֱ�Ӹ���ԭʼָ��޸�ƫ��
        newLocation[0] = instInfo->bytes[0]; // ����ԭʼ������
        newLocation[1] = (BYTE)pNewOffset;   // �����µ�ƫ����
        *newLength = 2; // ��������תָ���Ϊ2�ֽ�
    }
    else
    {
        // ƫ������������ת��Χ����Ҫת��Ϊ��������ת��ʹ����ת����
        // 1. ��ת��������ת����һ��ָ��֮����������תָ�
        newLocation[0] = (BYTE)(instInfo->bytes[0] ^ 0x01); // ��ת��������ת���λ��
        newLocation[1] = 0x06; // ��ת����һ��ָ��֮������6�ֽڵĽ���תָ�

        // 2. �������������ת��ԭʼĿ��
        newLocation[2] = 0xE9; // ����ת������
        *(INT32*)(newLocation + 3) = (INT32)(pOriginTargetAddr - ((UINT_PTR)newLocation + 7)); // ����ƫ����

        *newLength = 7; // �ܳ��ȣ�2�ֽ�������ת + 5�ֽ���������ת
    }

    return TRUE;
}

UINT_PTR InstructionRelocator::GetOriginTargetAddress(const InstructionInfo* instInfo)
{
    // ������ƫ�ƣ�����0
    if (!instInfo->displacement.isExists)
    {
        return 0;
    }

    // ����ԭʼĿ���ַ����ǰָ���ַ + ָ��� + ƫ������
    UINT_PTR pOriginTargetAddr = (UINT_PTR)instInfo->address + instInfo->length + instInfo->displacement.displacement;

    return pOriginTargetAddr;
}
