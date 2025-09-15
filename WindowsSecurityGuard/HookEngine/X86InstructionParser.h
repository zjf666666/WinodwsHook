#pragma once
#include "IInstructionParser.h"

class X86InstructionParser : public IInstructionParser
{
public:
    // ��������ָ�����ָ����Ϣ
    BOOL ParseInstruction(BYTE* codePtr, InstructionInfo* instInfo) override;

    // ��ȡָ������
    InstructionType GetInstructionType(const InstructionInfo* instInfo) override;

    // ����ָ���
    UINT GetInstructionLength(BYTE* codePtr) override;

    // ����Ƿ�Ϊ���Ѱַָ��
    BOOL IsRelativeInstruction(const InstructionInfo* instInfo) override;

    // ��ȡ���ѰַĿ���ַ
    BYTE* GetRelativeTargetAddress(const InstructionInfo* instInfo) override;

private:
    /*
     * @brief ����x86/x64ָ���ǰ׺�ֽ�
     * @param [IN] codePtr ָ��ǰҪ������ָ���ֽ����е�ָ��
     *        [IN/OUT] instInfo ָ����Ϣ�ṹ�壬���ڴ洢�������
     *                 ��������½ṹ������ǰ׺��ص��ֶ�
     * @return UINT ������ǰ׺ռ�õ��ֽ��������ڼ�����һ������λ��
     *              ���û��ǰ׺������0
     */
    UINT ParsePrefix(BYTE* codePtr, InstructionInfo* instInfo);

    // ����������
    UINT ParseOpcode(BYTE* codePtr, InstructionInfo* instInfo);

    // ����ModR/M�ֽ�
    UINT ParseModRM(BYTE* codePtr, InstructionInfo* instInfo);

    // ����SIB�ֽ�
    UINT ParseSIB(BYTE* codePtr, InstructionInfo* instInfo);

    // ����λ����
    UINT ParseDisplacement(BYTE* codePtr, InstructionInfo* instInfo);

    // ����������
    UINT ParseImmediate(BYTE* codePtr, InstructionInfo* instInfo);
};

