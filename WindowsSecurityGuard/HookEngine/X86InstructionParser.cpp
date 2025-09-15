#include "pch.h"
#include "X86InstructionParser.h"
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
#define OPCODE_SIZE_ONE                  1      // 操作码长度为1
#define OPCODE_SIZE_TWO                  2      // 操作码长度为2
#define OPCODE_SIZE_THREE                3      // 操作码长度为3
#define OPCODE_TYPE_EXTEND_3A            1      // 3字节操作码类型
#define OPCODE_TYPE_EXTEND_38            2      // 3字节操作码类型
#define OPCODE_EXTEND                    0x0F   // 扩展操作码
#define OPCODE_THREE_BYTE_38             0x38   // 3字节操作码38
#define OPCODE_THREE_BYTE_3A             0x3A   // 3字节操作码3A
#define OPCODE_TYPE_EXTEND               0      // 2字节操作码类型
#define OPCODE_TYPE_EXTEND_3A            1      // 3字节操作码类型
#define OPCODE_TYPE_EXTEND_38            2      // 3字节操作码类型

// ModR/M指令
#define MODRM_SIZE                       1      // ModR/M长度

// SIB指令
#define SIB_SIZE                       1        // SIB长度

BOOL X86InstructionParser::ParseInstruction(BYTE* codePtr, InstructionInfo* instInfo)
{
    // 非法值处理
    if (nullptr == codePtr)
    {
        Logger::GetInstance().Error(L"codePtr is NULL!");
        return false;
    }

    // 清理instInfo，避免有脏数据
    ZeroMemory(instInfo, sizeof(InstructionInfo));

    // 将指令地址、架构信息写入清理完后的结构体中
    instInfo->address = codePtr;
    instInfo->arch = ARCH_X86;

    // 解析指令前缀
    UINT uOffset = 0;
    uOffset = ParsePrefix(codePtr + uOffset, instInfo);

    // 解析操作码，操作码位于前缀之后
    uOffset += ParseOpcode(codePtr + uOffset, instInfo);

    // 解析ModR/M字节
    uOffset += ParseModRM(codePtr + uOffset, instInfo);

    if (3 != instInfo->modRM.mod && 4 == instInfo->modRM.rm)
    {
        uOffset += ParseSIB(codePtr + uOffset, instInfo);
    }

    return 0;
}

UINT X86InstructionParser::ParsePrefix(BYTE* codePtr, InstructionInfo* instInfo)
{
    // codePtr的空指针判断由调用者负责
    UINT uPriexSize = 0;
    BYTE byteCurrent;

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
