#include "pch.h"
#include "InstructionParser.h"
#include "../SecurityCore/Logger.h"
#include "../SecurityCore/MemoryUtils.h"

// 前缀指令
#define PREFIX_LOCK                      0xF0   // 锁定前缀
#define PREFIX_REPEAT_F2                 0xF2   // 重复前缀F2
#define PREFIX_REPEAT_F3                 0xF3   // 重复前缀F3
#define PREFIX_OPERAND                   0x66   // 操作数大小前缀
#define PREFIX_ADDRESS_SIZE              0x67   // 地址大小前缀
#define PREFIX_SEGMENT_REWRITE_26        0x26   // 段前缀26
#define PREFIX_SEGMENT_REWRITE_2E        0x2E   // 段前缀2E
#define PREFIX_SEGMENT_REWRITE_36        0x36   // 段前缀36
#define PREFIX_SEGMENT_REWRITE_3E        0x3E   // 段前缀3E
#define PREFIX_SEGMENT_REWRITE_64        0x64   // 段前缀64
#define PREFIX_SEGMENT_REWRITE_65        0x65   // 段前缀65

// 操作码指令
#define OPCODE_SIZE_ONE                         1      // 操作码长度为1
#define OPCODE_SIZE_TWO                         2      // 操作码长度为2
#define OPCODE_SIZE_THREE                       3      // 操作码长度为3
#define OPCODE_TYPE_EXTEND_3A                   1      // 3字节操作码类型
#define OPCODE_TYPE_EXTEND_38                   2      // 3字节操作码类型
#define OPCODE_EXTEND                           0x0F   // 扩展操作码
#define OPCODE_THREE_BYTE_38                    0x38   // 3字节操作码38
#define OPCODE_THREE_BYTE_3A                    0x3A   // 3字节操作码3A
#define OPCODE_TYPE_EXTEND                      0      // 2字节操作码类型
#define OPCODE_TYPE_EXTEND_3A                   1      // 3字节操作码类型
#define OPCODE_TYPE_EXTEND_38                   2      // 3字节操作码类型

// ModR/M指令
#define MODRM_SIZE                              1      // ModR/M长度
#define MOD_ADDRESSING_NO_OFFSET_OR_SPECIAL     0      // 0x00 无位移或特殊情况
#define MOD_ADDRESSING_8_OFFSET_MEMORY          1      // 0x01 带8位位移内存寻址
#define MOD_ADDRESSING_32_OFFSET_MEMORY         2      // 0x10 带32位位移内存寻址
#define MOD_ADDRESSING_REGISTER                 3      // 0x11 寄存器寻址
#define MOD_RM_REGISTER_OPERAND                 4      // 0x100 r/m指定一个寄存器操作数 其他情况是内存寻址
#define MOD_RM_SPECIAL_DIRECT_ADDRESSING        5      // 0x101 对应mod=00时：32位位移的直接寻址

// SIB指令
#define SIB_SIZE                                1        // SIB长度
#define SIB_BASE_B_BOX_101                      5        // 0x101 如果ModR/M.mod=00，则使用32位位移；否则为EBP

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
    // 非法值处理
    if (nullptr == codePtr || nullptr == instInfo)
    {
        Logger::GetInstance().Error(L"Ptr is NULL!");
        return FALSE;
    }

    // 清理instInfo，避免有脏数据
    ZeroMemory(instInfo, sizeof(InstructionInfo_study));

    // 将指令地址、架构信息写入清理完后的结构体中
    instInfo->address = codePtr;
    instInfo->arch = ARCH_X86;

    // 解析指令前缀
    UINT uOffset = 0;
    uOffset = ParsePrefix(codePtr + uOffset, instInfo);

    // 解析操作码，操作码位于前缀之后
    uOffset += ParseOpcode(codePtr + uOffset, instInfo);

    // 根据操作码，获取指令类型
    instInfo->type = GetInstructionType(instInfo);

    // 根据指令类型判断是否是相对寻址
    instInfo->isRelative = IsRelativeInstruction(instInfo);

    // 解析ModR/M字节
    uOffset += ParseModRM(codePtr + uOffset, instInfo);

    // 这个判断条件的详细说明见文档《ModRM与SIB字节关系解析.md》
    if (MOD_ADDRESSING_REGISTER != instInfo->modRM.mod && MOD_RM_REGISTER_OPERAND == instInfo->modRM.rm)
    {
        uOffset += ParseSIB(codePtr + uOffset, instInfo);
    }

    // 解析位移量
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

    // 获取主操作码
    BYTE opcode = instInfo->opcode.opcode;

    // 判断指令类型
    if (opcode == 0xE9)
    {
        return InstructionType::INST_JMP_NEAR;  // 近跳转
    }
    else if (opcode == 0xEB)
    {
        return InstructionType::INST_JMP_SHORT; // 短跳转
    }
    else if (opcode == 0xE8)
    {
        return InstructionType::INST_CALL_NEAR;  // 近调用
    }
    else if (opcode == 0xC3 || opcode == 0xC2)
    {
        return InstructionType::INST_RET;       // 返回指令
    }
    else if ((opcode >= 0x70 && opcode <= 0x7F) ||
        (instInfo->opcode.isExtended &&
            instInfo->opcode.extensionType == OPCODE_TYPE_EXTEND &&
            opcode >= 0x80 && opcode <= 0x8F))
    {
        return InstructionType::INST_COND_JMP;  // 条件跳转
    }

    return INST_NORMAL;  // 普通指令
}

BOOL InstructionParser::IsRelativeInstruction(const InstructionInfo_study* instInfo)
{
    BOOL bRes = FALSE;
    // 获取指令类型
    InstructionType type = GetInstructionType(instInfo);

    // 判断是否为相对寻址指令
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
    // codePtr的空指针判断由调用者负责
    UINT uPriexSize = 0;
    BYTE byteCurrent;

    bool bIsContinue = true; // 是否继续读取标志位

    // 前8位验证已经可以满足基本场景需求，由于这只是一个学习类，仅考虑前8位
    while (uPriexSize < 8) // 读取前8个字节
    {
        // 检测是否可读
        if (!MemoryUtils::IsMemoryReadable(codePtr + uPriexSize, 1))
        {
            Logger::GetInstance().Error(L"Read prefix failed! memory is not readable");
            break;
        }

        // 读取数据
        byteCurrent = codePtr[uPriexSize];

        // 判断是否为前缀
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
        // 检测扩展码
        if (OPCODE_EXTEND != *byteCurrent) // 非扩展，单字节操作码
        {
            // 保存操作码
            instInfo->opcode.opcode = *byteCurrent;
            instInfo->opcode.isExtended = false;
            uOpcodeLen = OPCODE_SIZE_ONE;
            break;
        }

        // 检测是否为3字节操作码
        ++byteCurrent;
        if (OPCODE_THREE_BYTE_38 == *byteCurrent ||
            OPCODE_THREE_BYTE_3A == *byteCurrent)
            // 使用3字节作为判断条件，这样便于后续扩展，因为这两个数是已确定为3字节操作码了
            // 其他情况只需在后面继续加判断条件即可，这里如果使用!=来判断，后续这个条件也需要改动
        {
            // 保存操作码
            BYTE byteTmp = *byteCurrent;
            ++byteCurrent;
            instInfo->opcode.opcode = *byteCurrent;
            instInfo->opcode.isExtended = true;
            instInfo->opcode.extensionType = OPCODE_THREE_BYTE_38 == byteTmp ?
                OPCODE_TYPE_EXTEND_38 : OPCODE_TYPE_EXTEND_3A;
            uOpcodeLen = OPCODE_SIZE_THREE;
            break;
        }

        // 其余清情况表示2操作数
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

    // 检查内存是否可读
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
    // 这里暂时不写宏了，后面看看有没有什么好点的处理方式
    switch (instInfo->opcode.opcode)
    {
        // 示例：mov reg, imm 指令
    case 0xB0: case 0xB1: case 0xB2: case 0xB3: // mov r8, imm8
    case 0xB4: case 0xB5: case 0xB6: case 0xB7:
        uImmediateSize = 1; // 8位立即数
        break;
    case 0xB8: case 0xB9: case 0xBA: case 0xBB: // mov r32, imm32
    case 0xBC: case 0xBD: case 0xBE: case 0xBF:
        // 检查操作数大小前缀
        if (instInfo->prefix.hasOperandSize) // 0x66前缀
        {
            uImmediateSize = 2; // 16位立即数
        }
        else
        {
            uImmediateSize = 4; // 32位立即数
        }
        break;
    default:
        break;
    }

    if (0 == uImmediateSize)
    {
        return 0;
    }

    // 检查内存是否可读
    if (!MemoryUtils::IsMemoryReadable(codePtr, uImmediateSize))
    {
        Logger::GetInstance().Error(L"Read immediate failed! memory is not readable");
        return 0;
    }

    // 保存立即数
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
        return 1; // 8位 1字节
    }

    if (MOD_ADDRESSING_32_OFFSET_MEMORY == instInfo->modRM.mod)
    {
        return 4; // 8位 1字节
    }

    if (MOD_ADDRESSING_NO_OFFSET_OR_SPECIAL == instInfo->modRM.mod)
    {
        if (MOD_RM_SPECIAL_DIRECT_ADDRESSING == instInfo->modRM.rm ||
            SIB_BASE_B_BOX_101 == instInfo->sib.base)
        {
            return 4; // 8位 1字节
        }
    }

    return 0;
}

