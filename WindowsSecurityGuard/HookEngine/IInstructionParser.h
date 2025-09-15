#pragma once

#include "InstructionTypes.h"

/* 
 * ָ��������ӿ��࣬�ⲿ�ֽ�����ѧϰ�ü���
 * ����ʹ�û�ѡ�������ѷ�װ�õĽ�����
 */
class IInstructionParser
{
public:
    virtual ~IInstructionParser() {}

    /*
     * @brief ����ָ���ڴ��ַ���ĵ���CPUָ���ȡ��ṹ��Ϣ
     * @param [IN] codePtr ָ��Ҫ������ָ���ֽ����е�ָ��
     *        [OUT] instInfo ���ڴ洢���������ָ����Ϣ�ṹ��ָ��
     * @return BOOL �ɹ���������TRUE��ʧ�ܷ���FALSE
     */
    virtual BOOL ParseInstruction(BYTE* codePtr, InstructionInfo* instInfo) = 0;

    // ��ȡָ������
    virtual InstructionType GetInstructionType(const InstructionInfo* instInfo) = 0;

    // ����ָ���
    virtual UINT GetInstructionLength(BYTE* codePtr) = 0;

    // ����Ƿ�Ϊ���Ѱַָ��
    virtual BOOL IsRelativeInstruction(const InstructionInfo* instInfo) = 0;

    // ��ȡ���ѰַĿ���ַ
    virtual BYTE* GetRelativeTargetAddress(const InstructionInfo* instInfo) = 0;
};