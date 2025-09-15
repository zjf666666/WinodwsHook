#include "pch.h"
#include "X86InstructionParser.h"
#include "../SecurityCore/Logger.h"
#include "../SecurityCore/MemoryUtils.h"

// ǰ׺ָ��
#define PREFIX_LOCK                      0xF0   // ����ǰ׺
#define PREFIX_REPEAT_F2                 0xF2   // �ظ�ǰ׺F2
#define PREFIX_REPEAT_F3                 0xF3   // �ظ�ǰ׺F3
#define PREFIX_OPERAND                   0x66   // ��������Сǰ׺
#define PREFIX_ADDRESS_SIZE              0x67   // ��ַ��Сǰ׺
#define PREFIX_SEGMENT_REWRITE_26        0x26   // ��ǰ׺26
#define PREFIX_SEGMENT_REWRITE_2E        0x2E   // ��ǰ׺2E
#define PREFIX_SEGMENT_REWRITE_36        0x36   // ��ǰ׺36
#define PREFIX_SEGMENT_REWRITE_3E        0x3E   // ��ǰ׺3E
#define PREFIX_SEGMENT_REWRITE_64        0x64   // ��ǰ׺64
#define PREFIX_SEGMENT_REWRITE_65        0x65   // ��ǰ׺65

// ������ָ��
#define OPCODE_SIZE_ONE                  1      // �����볤��Ϊ1
#define OPCODE_SIZE_TWO                  2      // �����볤��Ϊ2
#define OPCODE_SIZE_THREE                3      // �����볤��Ϊ3
#define OPCODE_TYPE_EXTEND_3A            1      // 3�ֽڲ���������
#define OPCODE_TYPE_EXTEND_38            2      // 3�ֽڲ���������
#define OPCODE_EXTEND                    0x0F   // ��չ������
#define OPCODE_THREE_BYTE_38             0x38   // 3�ֽڲ�����38
#define OPCODE_THREE_BYTE_3A             0x3A   // 3�ֽڲ�����3A
#define OPCODE_TYPE_EXTEND               0      // 2�ֽڲ���������
#define OPCODE_TYPE_EXTEND_3A            1      // 3�ֽڲ���������
#define OPCODE_TYPE_EXTEND_38            2      // 3�ֽڲ���������

// ModR/Mָ��
#define MODRM_SIZE                       1      // ModR/M����

// SIBָ��
#define SIB_SIZE                       1        // SIB����

BOOL X86InstructionParser::ParseInstruction(BYTE* codePtr, InstructionInfo* instInfo)
{
    // �Ƿ�ֵ����
    if (nullptr == codePtr)
    {
        Logger::GetInstance().Error(L"codePtr is NULL!");
        return false;
    }

    // ����instInfo��������������
    ZeroMemory(instInfo, sizeof(InstructionInfo));

    // ��ָ���ַ���ܹ���Ϣд���������Ľṹ����
    instInfo->address = codePtr;
    instInfo->arch = ARCH_X86;

    // ����ָ��ǰ׺
    UINT uOffset = 0;
    uOffset = ParsePrefix(codePtr + uOffset, instInfo);

    // ���������룬������λ��ǰ׺֮��
    uOffset += ParseOpcode(codePtr + uOffset, instInfo);

    // ����ModR/M�ֽ�
    uOffset += ParseModRM(codePtr + uOffset, instInfo);

    if (3 != instInfo->modRM.mod && 4 == instInfo->modRM.rm)
    {
        uOffset += ParseSIB(codePtr + uOffset, instInfo);
    }

    return 0;
}

UINT X86InstructionParser::ParsePrefix(BYTE* codePtr, InstructionInfo* instInfo)
{
    // codePtr�Ŀ�ָ���ж��ɵ����߸���
    UINT uPriexSize = 0;
    BYTE byteCurrent;

    // ǰ8λ��֤�Ѿ��������������������������ֻ��һ��ѧϰ�࣬������ǰ8λ
    while (uPriexSize < 8) // ��ȡǰ8���ֽ�
    {
        // ����Ƿ�ɶ�
        if (!MemoryUtils::IsMemoryReadable(codePtr + uPriexSize, 1))
        {
            Logger::GetInstance().Error(L"Read prefix failed! memory is not readable");
            break;
        }

        // ��ȡ����
        byteCurrent = codePtr[uPriexSize];

        // �ж��Ƿ�Ϊǰ׺
        switch (byteCurrent)
        {
        case PREFIX_LOCK:
            instInfo->prefix.hasLock = true;
            break;
        case PREFIX_REPEAT_F2:
        case PREFIX_REPEAT_F3:
            instInfo->prefix.repeat = byteCurrent;
            break;
        case PREFIX_OPERAND:
            instInfo->prefix.hasOperandSize = true;
            break;
        case PREFIX_ADDRESS_SIZE:
            instInfo->prefix.hasAddressSize = true;
            break;
        case PREFIX_SEGMENT_REWRITE_26:
        case PREFIX_SEGMENT_REWRITE_2E:
        case PREFIX_SEGMENT_REWRITE_36:
        case PREFIX_SEGMENT_REWRITE_3E:
        case PREFIX_SEGMENT_REWRITE_64:
        case PREFIX_SEGMENT_REWRITE_65:
            instInfo->prefix.segment = byteCurrent;
            break;
        default:
            break;
        }
        instInfo->bytes[uPriexSize] = byteCurrent;
        ++uPriexSize;
    }
    instInfo->prefix.size = uPriexSize;
    return uPriexSize;
}

UINT X86InstructionParser::ParseOpcode(BYTE* codePtr, InstructionInfo* instInfo)
{
    BYTE* byteCurrent = codePtr;
    UINT uOpcodeLen = 0;

    do
    {
        if (!MemoryUtils::IsMemoryReadable(codePtr, 1))
        {
            Logger::GetInstance().Error(L"Read opcode failed! memory is not readable");
            break;
        }
        // �����չ��
        if (OPCODE_EXTEND != *byteCurrent) // ����չ�����ֽڲ�����
        {
            // ���������
            instInfo->opcode.opcode = *byteCurrent;
            instInfo->opcode.isExtended = false;
            uOpcodeLen = OPCODE_SIZE_ONE;
            break;
        }

        // ����Ƿ�Ϊ3�ֽڲ�����
        ++byteCurrent;
        if (OPCODE_THREE_BYTE_38 == *byteCurrent ||
            OPCODE_THREE_BYTE_3A == *byteCurrent) 
        // ʹ��3�ֽ���Ϊ�ж��������������ں�����չ����Ϊ������������ȷ��Ϊ3�ֽڲ�������
        // �������ֻ���ں���������ж��������ɣ��������ʹ��!=���жϣ������������Ҳ��Ҫ�Ķ�
        {
            // ���������
            BYTE byteTmp = *byteCurrent;
            ++byteCurrent;
            instInfo->opcode.opcode = *byteCurrent;
            instInfo->opcode.isExtended = true;
            instInfo->opcode.extensionType = OPCODE_THREE_BYTE_38 == byteTmp ?
                OPCODE_TYPE_EXTEND_38 : OPCODE_TYPE_EXTEND_3A;
            uOpcodeLen = OPCODE_SIZE_THREE;
            break;
        }

        // �����������ʾ2������
        instInfo->opcode.opcode = *byteCurrent;
        instInfo->opcode.isExtended = true;
        instInfo->opcode.extensionType = OPCODE_TYPE_EXTEND;
        uOpcodeLen = OPCODE_SIZE_TWO;
        break;
    } while (false);

    return uOpcodeLen;
}

UINT X86InstructionParser::ParseModRM(BYTE* codePtr, InstructionInfo* instInfo)
{
    if (!MemoryUtils::IsMemoryReadable(codePtr, 1))
    {
        Logger::GetInstance().Error(L"Read modr/m failed! memory is not readable");
        return 0;
    }

    instInfo->modRM.modRM = *codePtr;
    instInfo->modRM.mod   = (instInfo->modRM.modRM >> 6) & 0x03;
    instInfo->modRM.reg = (instInfo->modRM.modRM >> 3) & 0x07;
    instInfo->modRM.rm = instInfo->modRM.modRM & 0x07;
    return MODRM_SIZE;
}

UINT X86InstructionParser::ParseSIB(BYTE* codePtr, InstructionInfo* instInfo)
{
    if (!MemoryUtils::IsMemoryReadable(codePtr, 1))
    {
        Logger::GetInstance().Error(L"Read sib failed! memory is not readable");
        return 0;
    }

    instInfo->sib.sib = *codePtr;
    instInfo->sib.scale = (instInfo->sib.sib >> 6) & 0x03;
    instInfo->sib.index = (instInfo->sib.sib >> 3) & 0x07;
    instInfo->sib.base = instInfo->sib.sib & 0x07;

    return 0;
}
