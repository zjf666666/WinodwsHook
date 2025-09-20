#include "pch.h"
#include "InstructionParser.h"
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
#define OPCODE_SIZE_ONE                         1      // �����볤��Ϊ1
#define OPCODE_SIZE_TWO                         2      // �����볤��Ϊ2
#define OPCODE_SIZE_THREE                       3      // �����볤��Ϊ3
#define OPCODE_TYPE_EXTEND_3A                   1      // 3�ֽڲ���������
#define OPCODE_TYPE_EXTEND_38                   2      // 3�ֽڲ���������
#define OPCODE_EXTEND                           0x0F   // ��չ������
#define OPCODE_THREE_BYTE_38                    0x38   // 3�ֽڲ�����38
#define OPCODE_THREE_BYTE_3A                    0x3A   // 3�ֽڲ�����3A
#define OPCODE_TYPE_EXTEND                      0      // 2�ֽڲ���������
#define OPCODE_TYPE_EXTEND_3A                   1      // 3�ֽڲ���������
#define OPCODE_TYPE_EXTEND_38                   2      // 3�ֽڲ���������

// ModR/Mָ��
#define MODRM_SIZE                              1      // ModR/M����
#define MOD_ADDRESSING_NO_OFFSET_OR_SPECIAL     0      // 0x00 ��λ�ƻ��������
#define MOD_ADDRESSING_8_OFFSET_MEMORY          1      // 0x01 ��8λλ���ڴ�Ѱַ
#define MOD_ADDRESSING_32_OFFSET_MEMORY         2      // 0x10 ��32λλ���ڴ�Ѱַ
#define MOD_ADDRESSING_REGISTER                 3      // 0x11 �Ĵ���Ѱַ
#define MOD_RM_REGISTER_OPERAND                 4      // 0x100 r/mָ��һ���Ĵ��������� ����������ڴ�Ѱַ
#define MOD_RM_SPECIAL_DIRECT_ADDRESSING        5      // 0x101 ��Ӧmod=00ʱ��32λλ�Ƶ�ֱ��Ѱַ

// SIBָ��
#define SIB_SIZE                                1        // SIB����
#define SIB_BASE_B_BOX_101                      5        // 0x101 ���ModR/M.mod=00����ʹ��32λλ�ƣ�����ΪEBP

BOOL InstructionParser::ParseInstruction(BYTE* codePtr, InstructionInfo_study* instInfo, InstructionArchitecture arch)
{
    BOOL bRes = FALSE;
    switch (arch)
    {
    case ARCH_X86:
        bRes = ParseInstructionX86(codePtr, instInfo);
        break;
    case ARCH_X64:
        bRes = ParseInstructionX64(codePtr, instInfo);
        break;
    default:
        break;
    }
    return bRes;
}

BOOL InstructionParser::ParseInstructionX86(BYTE* codePtr, InstructionInfo_study* instInfo)
{
    // �Ƿ�ֵ����
    if (nullptr == codePtr || nullptr == instInfo)
    {
        Logger::GetInstance().Error(L"Ptr is NULL!");
        return FALSE;
    }

    // ����instInfo��������������
    ZeroMemory(instInfo, sizeof(InstructionInfo_study));

    // ��ָ���ַ���ܹ���Ϣд���������Ľṹ����
    instInfo->address = codePtr;
    instInfo->arch = ARCH_X86;

    // ����ָ��ǰ׺
    UINT uOffset = 0;
    uOffset = ParsePrefix(codePtr + uOffset, instInfo);

    // ���������룬������λ��ǰ׺֮��
    uOffset += ParseOpcode(codePtr + uOffset, instInfo);

    // ���ݲ����룬��ȡָ������
    instInfo->type = GetInstructionType(instInfo);

    // ����ָ�������ж��Ƿ������Ѱַ
    instInfo->isRelative = IsRelativeInstruction(instInfo);

    // ����ModR/M�ֽ�
    uOffset += ParseModRM(codePtr + uOffset, instInfo);

    // ����ж���������ϸ˵�����ĵ���ModRM��SIB�ֽڹ�ϵ����.md��
    if (MOD_ADDRESSING_REGISTER != instInfo->modRM.mod && MOD_RM_REGISTER_OPERAND == instInfo->modRM.rm)
    {
        uOffset += ParseSIB(codePtr + uOffset, instInfo);
    }

    // ����λ����
    uOffset += ParseDisplacement(codePtr + uOffset, instInfo);
    if (MOD_ADDRESSING_REGISTER != instInfo->modRM.mod && instInfo->displacement.displacement > 0)
    {
        instInfo->modRM.isRelative = TRUE;
    }

    uOffset += ParseImmediate(codePtr + uOffset, instInfo);

    instInfo->length = uOffset;
    instInfo->type = GetInstructionType(instInfo);
    return TRUE;
}

BOOL InstructionParser::ParseInstructionX64(BYTE* codePtr, InstructionInfo_study* instInfo)
{
    return 0;
}

InstructionType InstructionParser::GetInstructionType(const InstructionInfo_study* instInfo)
{
    if (!instInfo)
    {
        return InstructionType::INST_UNKNOWN;
    }

    // ��ȡ��������
    BYTE opcode = instInfo->opcode.opcode;

    // �ж�ָ������
    if (opcode == 0xE9)
    {
        return InstructionType::INST_JMP_NEAR;  // ����ת
    }
    else if (opcode == 0xEB)
    {
        return InstructionType::INST_JMP_SHORT; // ����ת
    }
    else if (opcode == 0xE8)
    {
        return InstructionType::INST_CALL_NEAR;  // ������
    }
    else if (opcode == 0xC3 || opcode == 0xC2)
    {
        return InstructionType::INST_RET;       // ����ָ��
    }
    else if ((opcode >= 0x70 && opcode <= 0x7F) ||
        (instInfo->opcode.isExtended &&
            instInfo->opcode.extensionType == OPCODE_TYPE_EXTEND &&
            opcode >= 0x80 && opcode <= 0x8F))
    {
        return InstructionType::INST_COND_JMP;  // ������ת
    }

    return INST_NORMAL;  // ��ָͨ��
}

BOOL InstructionParser::IsRelativeInstruction(const InstructionInfo_study* instInfo)
{
    BOOL bRes = FALSE;
    // ��ȡָ������
    InstructionType type = GetInstructionType(instInfo);

    // �ж��Ƿ�Ϊ���Ѱַָ��
    switch (type)
    {
    case INST_JMP_NEAR:
    case INST_JMP_SHORT:
    case INST_CALL_NEAR:
    case INST_COND_JMP:
        bRes = TRUE;
        break;
    default:
        break;
    }
    return bRes;
}

UINT InstructionParser::ParsePrefix(BYTE* codePtr, InstructionInfo_study* instInfo)
{
    // codePtr�Ŀ�ָ���ж��ɵ����߸���
    UINT uPriexSize = 0;
    BYTE byteCurrent;

    bool bIsContinue = true; // �Ƿ������ȡ��־λ

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
            bIsContinue = false;
            break;
        }
        instInfo->bytes[uPriexSize] = byteCurrent;
        ++uPriexSize;
    }
    instInfo->prefix.size = uPriexSize;
    if (0 != uPriexSize)
    {
        instInfo->prefix.isExists = TRUE;
    }
    return uPriexSize;
}

UINT InstructionParser::ParseOpcode(BYTE* codePtr, InstructionInfo_study* instInfo)
{
    BYTE* byteCurrent = codePtr;
    UINT uOpcodeLen = 0;

    if (!MemoryUtils::IsMemoryReadable(codePtr, 1))
    {
        Logger::GetInstance().Error(L"Read opcode failed! memory is not readable");
        return 0;
    }

    do
    {
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
    instInfo->opcode.size = uOpcodeLen;
    instInfo->opcode.isExists = TRUE;
    return uOpcodeLen;
}

UINT InstructionParser::ParseModRM(BYTE* codePtr, InstructionInfo_study* instInfo)
{
    if (!MemoryUtils::IsMemoryReadable(codePtr, 1))
    {
        Logger::GetInstance().Error(L"Read modr/m failed! memory is not readable");
        return 0;
    }

    instInfo->modRM.isExists = TRUE;
    instInfo->modRM.size = MODRM_SIZE;
    instInfo->modRM.modRM = *codePtr;
    instInfo->modRM.mod = (instInfo->modRM.modRM >> 6) & 0x03;
    instInfo->modRM.reg = (instInfo->modRM.modRM >> 3) & 0x07;
    instInfo->modRM.rm = instInfo->modRM.modRM & 0x07;
    return MODRM_SIZE;
}

UINT InstructionParser::ParseSIB(BYTE* codePtr, InstructionInfo_study* instInfo)
{
    if (!MemoryUtils::IsMemoryReadable(codePtr, 1))
    {
        Logger::GetInstance().Error(L"Read sib failed! memory is not readable");
        return 0;
    }

    instInfo->sib.isExists = TRUE;
    instInfo->sib.size = SIB_SIZE;
    instInfo->sib.sib = *codePtr;
    instInfo->sib.scale = (instInfo->sib.sib >> 6) & 0x03;
    instInfo->sib.index = (instInfo->sib.sib >> 3) & 0x07;
    instInfo->sib.base = instInfo->sib.sib & 0x07;

    return SIB_SIZE;
}

UINT InstructionParser::ParseDisplacement(BYTE* codePtr, InstructionInfo_study* instInfo)
{
    UINT uDisplacementSize = GetDisplacementSize(instInfo);

    if (0 == uDisplacementSize)
    {
        return 0;
    }

    // ����ڴ��Ƿ�ɶ�
    if (!MemoryUtils::IsMemoryReadable(codePtr, uDisplacementSize))
    {
        Logger::GetInstance().Error(L"Read displacement failed! memory is not readable");
        return 0;
    }

    if (1 == uDisplacementSize)
    {
        instInfo->displacement.displacement = *(INT8*)codePtr;
    }
    else
    {
        instInfo->displacement.displacement = *(INT32*)codePtr;
    }
    instInfo->displacement.isExists = TRUE;
    instInfo->displacement.size = uDisplacementSize;
    return uDisplacementSize;
}

UINT InstructionParser::ParseImmediate(BYTE* codePtr, InstructionInfo_study* instInfo)
{
    UINT uImmediateSize = 0;
    // ������ʱ��д���ˣ����濴����û��ʲô�õ�Ĵ���ʽ
    switch (instInfo->opcode.opcode)
    {
        // ʾ����mov reg, imm ָ��
    case 0xB0: case 0xB1: case 0xB2: case 0xB3: // mov r8, imm8
    case 0xB4: case 0xB5: case 0xB6: case 0xB7:
        uImmediateSize = 1; // 8λ������
        break;
    case 0xB8: case 0xB9: case 0xBA: case 0xBB: // mov r32, imm32
    case 0xBC: case 0xBD: case 0xBE: case 0xBF:
        // ����������Сǰ׺
        if (instInfo->prefix.hasOperandSize) // 0x66ǰ׺
        {
            uImmediateSize = 2; // 16λ������
        }
        else
        {
            uImmediateSize = 4; // 32λ������
        }
        break;
    default:
        break;
    }

    if (0 == uImmediateSize)
    {
        return 0;
    }

    // ����ڴ��Ƿ�ɶ�
    if (!MemoryUtils::IsMemoryReadable(codePtr, uImmediateSize))
    {
        Logger::GetInstance().Error(L"Read immediate failed! memory is not readable");
        return 0;
    }

    // ����������
    if (1 == uImmediateSize)
    {
        instInfo->immediate.immediate = *(INT8*)codePtr;
    }
    else if (2 == uImmediateSize)
    {
        instInfo->immediate.immediate = *(INT16*)codePtr;
    }
    else if (4 == uImmediateSize)
    {
        instInfo->immediate.immediate = *(INT32*)codePtr;
    }
    instInfo->immediate.isExists = TRUE;
    instInfo->immediate.size = uImmediateSize;
    return uImmediateSize;
}

UINT InstructionParser::GetDisplacementSize(InstructionInfo_study* instInfo)
{
    if (MOD_ADDRESSING_8_OFFSET_MEMORY == instInfo->modRM.mod)
    {
        return 1; // 8λ 1�ֽ�
    }

    if (MOD_ADDRESSING_32_OFFSET_MEMORY == instInfo->modRM.mod)
    {
        return 4; // 8λ 1�ֽ�
    }

    if (MOD_ADDRESSING_NO_OFFSET_OR_SPECIAL == instInfo->modRM.mod)
    {
        if (MOD_RM_SPECIAL_DIRECT_ADDRESSING == instInfo->modRM.rm ||
            SIB_BASE_B_BOX_101 == instInfo->sib.base)
        {
            return 4; // 8λ 1�ֽ�
        }
    }

    return 0;
}

