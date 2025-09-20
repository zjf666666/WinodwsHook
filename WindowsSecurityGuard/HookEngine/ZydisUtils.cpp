#include "pch.h"
#include "ZydisUtils.h"

#include <Zydis/Zydis.h>

#include "../SecurityCore/Logger.h"
#include "../SecurityCore/MemoryUtils.h"

#define NEED_SIZE_64      12  // 64位跳转指令长度
#define NEED_SIZE_32      5   // 32位跳转指令长度
#define NEED_SIZE_8       2   // 8位跳转指令长度

struct ZydisContext
{
    ZydisDecoder decoder;
    ZydisDecodedInstruction decodedInstInfo;
    ZydisDecodedOperand decodedOperandInfo[ZYDIS_MAX_OPERAND_COUNT];
    InstructionType type; // 指令类型
    bool isRelative; // 是否为相对跳转或调用指令，true表示需要重定位处理
};

// 创建上下文：初始化Zydis资源，返回不透明指针
ZydisContextPtr ZydisUtils::CreateContext(bool is64Bit)
{
    // 堆上分配内部结构（避免栈内存过大）
    ZydisContext* context = (ZydisContext*)malloc(sizeof(ZydisContext));
    if (nullptr == context)
    {
        return nullptr;
    }

    // 初始化解码器
    ZydisMachineMode zyMachineMode = ZYDIS_MACHINE_MODE_LONG_64;
    ZydisStackWidth zyStackWidth = ZYDIS_STACK_WIDTH_64;
    if (!is64Bit) // 根据系统架构选择对应的栈宽及机器类型
    {
        zyMachineMode = ZYDIS_MACHINE_MODE_LEGACY_32;
        zyStackWidth = ZYDIS_STACK_WIDTH_32;
    }
    int nRes = ZydisDecoderInit(&context->decoder, zyMachineMode, zyStackWidth);
    if (ZYAN_STATUS_SUCCESS != nRes) // 初始化失败，返回false
    {
        Logger::GetInstance().Error(L"ZydisDecoderInit failed! error = %d", nRes);
        return nullptr;
    }
    return context;
}

// 销毁上下文：释放资源
void ZydisUtils::DestroyContext(ZydisContextPtr context)
{
    if (nullptr != context)
    {
        free(context); // 释放堆内存
    }
}

bool ZydisUtils::ParseInstruction(
    const void* instructionBytes,
    size_t length,
    size_t* parseLength,  // 实际解析长度
    ZydisContextPtr zyContext
)
{
    if (nullptr == zyContext)
    {
        Logger::GetInstance().Error(L"zyContext is nullptr!");
        return false;
    }

    size_t size = min(length, 20); // 不超过20字节
    int nRes = ZydisDecoderDecodeFull(&zyContext->decoder, instructionBytes, size, &zyContext->decodedInstInfo, zyContext->decodedOperandInfo);
    if (ZYAN_STATUS_SUCCESS != nRes)
    {
        Logger::GetInstance().Error(L"ZydisDecoderDecodeFull failed! error = %d", nRes);
        return false;
    }

    zyContext->type = GetInstructionType(zyContext);
    zyContext->isRelative = IsRelativeInstruction(zyContext);
    *parseLength = zyContext->decodedInstInfo.length;
    return true;
}

bool ZydisUtils::RelocateInstruction(
    ZydisContextPtr zyData,
    BYTE* sourceAddress,
    BYTE* targetAddress,
    size_t* bufferSize,
    size_t* outputSize
)
{
    // bufferSize指针为空，表示计算长度
    if (nullptr == zyData || nullptr == sourceAddress || nullptr == targetAddress || nullptr == outputSize)
    {
        Logger::GetInstance().Error(L"Invalid parameters!");
        return false;
    }

    ZydisDecodedInstruction zyInstInfo = zyData->decodedInstInfo;
    if (!MemoryUtils::IsMemoryReadable((BYTE*)sourceAddress, zyInstInfo.length))
    {
        Logger::GetInstance().Error(L"Source instruction memory is not readable!");
        return false;
    }

    bool bRes = true;

    switch (zyData->type)
    {
    case INST_JMP_NEAR:
        bRes = RelocateRelativeJump(
                    zyData,
                    sourceAddress,
                    targetAddress,
                    bufferSize,
                    outputSize
                );
        break;
    case INST_JMP_FAR:
        RelocateAbsoluteJump();
        break;
    case INST_JMP_INDIRECT:
        RelocateIndirectJump();
        break;
    case INST_JCC:
        RelocateConditionalJump();
        break;
    case INST_CALL_NEAR:
        RelocateRelativeCall();
        break;
    case INST_CALL_FAR:
        RelocateAbsoluteCall();
        break;
    case INST_RET:
    case INST_SYSCALL:
    case INST_INT:
    case INST_NORMAL:
        CopyInstruction();
        break;
    default:
        bRes = false;
        break;
    }
    return bRes;
}

InstructionType ZydisUtils::GetInstructionType(const ZydisContextPtr zyContext)
{
    if (nullptr == zyContext)
    {
        Logger::GetInstance().Error(L"zyContext is nullptr!");
        return InstructionType::INST_UNKNOWN;
    }

    ZydisDecodedInstruction decodedInst = zyContext->decodedInstInfo;

    InstructionType type = InstructionType::INST_UNKNOWN;

    // 根据助记符判断指令类型
    switch (decodedInst.mnemonic)
    {
        // 无条件跳转指令
    case ZYDIS_MNEMONIC_JMP:
        // 判断是否为近跳转（相对地址）
        if (decodedInst.meta.branch_type == ZYDIS_BRANCH_TYPE_SHORT ||
            decodedInst.meta.branch_type == ZYDIS_BRANCH_TYPE_NEAR)
        {
            type = InstructionType::INST_JMP_NEAR;
        }
        // 判断是否为远跳转（绝对地址）
        else if (decodedInst.meta.branch_type == ZYDIS_BRANCH_TYPE_FAR)
        {
            type = InstructionType::INST_JMP_FAR;
        }
        else
        {
            type = INST_JMP_INDIRECT;
        }
        break;
        // 条件跳转指令
    case ZYDIS_MNEMONIC_JZ:
    case ZYDIS_MNEMONIC_JNZ:
    case ZYDIS_MNEMONIC_JB:
    case ZYDIS_MNEMONIC_JNB:
    case ZYDIS_MNEMONIC_JBE:
    case ZYDIS_MNEMONIC_JNBE:
    case ZYDIS_MNEMONIC_JL:
    case ZYDIS_MNEMONIC_JNL:
    case ZYDIS_MNEMONIC_JLE:
    case ZYDIS_MNEMONIC_JNLE:
    case ZYDIS_MNEMONIC_JO:
    case ZYDIS_MNEMONIC_JNO:
    case ZYDIS_MNEMONIC_JP:
    case ZYDIS_MNEMONIC_JNP:
    case ZYDIS_MNEMONIC_JS:
    case ZYDIS_MNEMONIC_JNS:
    case ZYDIS_MNEMONIC_JCXZ:
    case ZYDIS_MNEMONIC_JECXZ:
    case ZYDIS_MNEMONIC_JRCXZ:
        type = INST_JCC;
        break;
        // 调用指令
    case ZYDIS_MNEMONIC_CALL:
        if (decodedInst.meta.branch_type == ZYDIS_BRANCH_TYPE_NEAR)
        {
            type = INST_CALL_NEAR;
        }
        else
        {
            type = INST_CALL_FAR;
        }
        break;
        // 返回指令
    case ZYDIS_MNEMONIC_RET:
    case ZYDIS_MNEMONIC_IRET:
    case ZYDIS_MNEMONIC_IRETD:
    case ZYDIS_MNEMONIC_IRETQ:
        type = INST_RET;
        break;
        // 系统调用指令
    case ZYDIS_MNEMONIC_SYSCALL:
    case ZYDIS_MNEMONIC_SYSENTER:
    case ZYDIS_MNEMONIC_SYSEXIT:
    case ZYDIS_MNEMONIC_SYSRET:
        type = INST_SYSCALL;
        break;
        // 中断指令
    case ZYDIS_MNEMONIC_INT:
    case ZYDIS_MNEMONIC_INT1:
    case ZYDIS_MNEMONIC_INT3:
    case ZYDIS_MNEMONIC_INTO:
        type = INST_INT;
        break;
    default:
        type = INST_NORMAL;
        break;
    }
    return type;
}

bool ZydisUtils::IsRelativeInstruction(const ZydisContextPtr zyContext)
{
    if (nullptr == zyContext)
    {
        Logger::GetInstance().Error(L"zyContext is nullptr!");
        return false;
    }

    // 检查是否为相对跳转或调用指令
    if ((ZYDIS_BRANCH_TYPE_SHORT == zyContext->decodedInstInfo.meta.branch_type ||
         ZYDIS_BRANCH_TYPE_NEAR == zyContext->decodedInstInfo.meta.branch_type) &&
        (ZYDIS_CATEGORY_COND_BR == zyContext->decodedInstInfo.meta.category ||
         ZYDIS_CATEGORY_UNCOND_BR == zyContext->decodedInstInfo.meta.category ||
         ZYDIS_CATEGORY_CALL == zyContext->decodedInstInfo.meta.category))
    {
        return true;
    }

    // 检查是否为RIP相对寻址指令
    if (ZYDIS_MACHINE_MODE_LONG_64 != zyContext->decodedInstInfo.machine_mode)
    {
        return false;
    }

    for (int i = 0; i < zyContext->decodedInstInfo.operand_count; i++)
    {
        if (ZYDIS_OPERAND_TYPE_MEMORY == zyContext->decodedOperandInfo[i].type && ZYDIS_REGISTER_RIP == zyContext->decodedOperandInfo[i].mem.base)
        {
            return true;
        }
    }
    return false;
}

bool ZydisUtils::RelocateRelativeJump(
    ZydisContextPtr zyData,
    BYTE* sourceAddress,
    BYTE* targetAddress,
    size_t* bufferSize,
    size_t* outputSize
)
{
    // 指针有效性由调用方控制，sourceAddress内存可读性也由调用方保证
    // targetAddress由于指令长度可能发生改变，因此地址空间是否足够在函数内部进行检测
    // 如果bufferSize为空，则返回需要的长度
    ZydisDecodedInstruction decodedInst = zyData->decodedInstInfo;
    ZydisDecodedOperand decodeOperand = zyData->decodedOperandInfo[0];

    // 计算原始目标地址 偏移 + 指令长度 + 当前地址
    UINT_PTR absoluteAddr = (UINT_PTR)sourceAddress + decodeOperand.imm.value.s + zyData->decodedInstInfo.length;

    // 计算新的偏移大小 目标地址 - 指令长度 - 存放地址
    INT64 newOffset = (INT32)((UINT_PTR)absoluteAddr - (UINT_PTR)targetAddress - zyData->decodedInstInfo.length);


    // 计算长度
    bool bIs64Bit = ZYDIS_MACHINE_MODE_LONG_64 == decodedInst.machine_mode;
    do
    {
        if (newOffset > INT32_MAX || newOffset < INT32_MIN)
        {
            // 64位长度仅支持x64
            if (ZYDIS_MACHINE_MODE_LONG_64 != decodedInst.machine_mode)
            {
                Logger::GetInstance().Error(L"Excessive offset!");
                *outputSize = 0;
                return false;
            }
            *outputSize = NEED_SIZE_64;
            break;
        }

        if (newOffset > INT8_MAX || newOffset < INT8_MIN)
        {
            *outputSize = NEED_SIZE_32;
            break;
        }

        *outputSize = NEED_SIZE_8;
    } while (false);

    if (nullptr == bufferSize)
    {
        return true;
    }

    // 日志在函数内部记录
    if (!CheckRelativeJumpParam(targetAddress, bufferSize, *outputSize))
    {
        return false;
    }

    // 写入指令
    do
    {
        if (NEED_SIZE_64 == *outputSize)
        {
            // 生成64位跳转指令
            targetAddress[0] = 0x48;  // REX.W
            targetAddress[1] = 0xB8;  // MOV RAX, imm64
            memcpy(targetAddress + 2, &absoluteAddr, sizeof(absoluteAddr));
            targetAddress[10] = 0xFF; // JMP RAX
            targetAddress[11] = 0xE0;
            break;
        }

        if (NEED_SIZE_32 == *outputSize)
        {
            targetAddress[0] = 0xE9;
            memcpy(&newOffset, targetAddress + 1, sizeof(INT32));
            break;
        }

        targetAddress[0] = 0xEB;
        targetAddress[1] = newOffset;
    } while (false);

    return true;
}

bool ZydisUtils::RelocateAbsoluteJump()
{
    return false;
}

bool ZydisUtils::RelocateIndirectJump( )
{
    return false;
}

bool ZydisUtils::RelocateConditionalJump( )
{
    return false;
}

bool ZydisUtils::RelocateRelativeCall( )
{
    return false;
}

bool ZydisUtils::RelocateAbsoluteCall( )
{
    return false;
}

bool ZydisUtils::CopyInstruction( )
{
    return false;
}

bool ZydisUtils::CheckRelativeJumpParam(
    BYTE* targetAddress,
    size_t* bufferSize,
    size_t needSize
)
{
    // 缓冲区长度是否足够
    if (*bufferSize < needSize)
    {
        Logger::GetInstance().Error(L"Target address has insufficient memory!");
        return false;
    }

    // 判断可写长度是否充足
    if (!MemoryUtils::IsMemoryWritable(targetAddress, needSize))
    {
        Logger::GetInstance().Error(L"Target memory is not writeable!");
        return false;
    }

    return true;
}