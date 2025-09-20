#pragma once

#include <Windows.h>

#include "InstructionTypes.h"

/*
 * �Ѵ����˽�Ϊʲô��Ҫ����ָ����ʵ����x86�ܹ���ָ�������ʽ
 * ���������ݲ�ʵ�֣�ʹ��Zydis����ʵ���������
 */

class InstructionParser
{
public:
    static BOOL ParseInstruction(BYTE* codePtr, InstructionInfo_study* instInfo, InstructionArchitecture arch);

private:
    static BOOL ParseInstructionX86(BYTE* codePtr, InstructionInfo_study* instInfo);

    static BOOL ParseInstructionX64(BYTE* codePtr, InstructionInfo_study* instInfo);

private:
    // ��ȡָ������
    static InstructionType GetInstructionType(const InstructionInfo_study* instInfo);

    // ����Ƿ�Ϊ���Ѱַָ��
    static BOOL IsRelativeInstruction(const InstructionInfo_study* instInfo);

    /*
     * @brief ����x86/x64ָ���ǰ׺�ֽ�
     * @param [IN] codePtr ָ��ǰҪ������ָ���ֽ����е�ָ��
     *        [IN/OUT] instInfo ָ����Ϣ�ṹ�壬���ڴ洢�������
     *                 ��������½ṹ������ǰ׺��ص��ֶ�
     * @return UINT ������ǰ׺ռ�õ��ֽ��������ڼ�����һ������λ��
     *              ���û��ǰ׺������0
     */
    static UINT ParsePrefix(BYTE* codePtr, InstructionInfo_study* instInfo);

    // ����������
    static UINT ParseOpcode(BYTE* codePtr, InstructionInfo_study* instInfo);

    // ����ModR/M�ֽ�
    static UINT ParseModRM(BYTE* codePtr, InstructionInfo_study* instInfo);

    // ����SIB�ֽ�
    static UINT ParseSIB(BYTE* codePtr, InstructionInfo_study* instInfo);

    // ����λ����
    static UINT ParseDisplacement(BYTE* codePtr, InstructionInfo_study* instInfo);

    // ����������
    static UINT ParseImmediate(BYTE* codePtr, InstructionInfo_study* instInfo);

    // ��ȡλ������С
    static UINT GetDisplacementSize(InstructionInfo_study* instInfo);
};

