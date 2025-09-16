#pragma once

#include <Windows.h>

#include "InstructionTypes.h"

class InstructionParser
{
public:
    static BOOL ParseInstruction(BYTE* codePtr, InstructionInfo* instInfo, InstructionArchitecture arch);

private:
    static BOOL ParseInstructionX86(BYTE* codePtr, InstructionInfo* instInfo);

    static BOOL ParseInstructionX64(BYTE* codePtr, InstructionInfo* instInfo);

private:
    // ��ȡָ������
    static InstructionType GetInstructionType(const InstructionInfo* instInfo);

    // ����Ƿ�Ϊ���Ѱַָ��
    static BOOL IsRelativeInstruction(const InstructionInfo* instInfo);

    /*
     * @brief ����x86/x64ָ���ǰ׺�ֽ�
     * @param [IN] codePtr ָ��ǰҪ������ָ���ֽ����е�ָ��
     *        [IN/OUT] instInfo ָ����Ϣ�ṹ�壬���ڴ洢�������
     *                 ��������½ṹ������ǰ׺��ص��ֶ�
     * @return UINT ������ǰ׺ռ�õ��ֽ��������ڼ�����һ������λ��
     *              ���û��ǰ׺������0
     */
    static UINT ParsePrefix(BYTE* codePtr, InstructionInfo* instInfo);

    // ����������
    static UINT ParseOpcode(BYTE* codePtr, InstructionInfo* instInfo);

    // ����ModR/M�ֽ�
    static UINT ParseModRM(BYTE* codePtr, InstructionInfo* instInfo);

    // ����SIB�ֽ�
    static UINT ParseSIB(BYTE* codePtr, InstructionInfo* instInfo);

    // ����λ����
    static UINT ParseDisplacement(BYTE* codePtr, InstructionInfo* instInfo);

    // ����������
    static UINT ParseImmediate(BYTE* codePtr, InstructionInfo* instInfo);

    // ��ȡλ������С
    static UINT GetDisplacementSize(InstructionInfo* instInfo);
};

