#include "pch.h"
#include "ZydisUtils.h"

#include <Zydis/Zydis.h>
#include <intrin.h>

#include "../SecurityCore/Logger.h"
#include "../SecurityCore/MemoryUtils.h"

#define NEED_SIZE_64      12  // 64位跳转指令长度
#define NEED_SIZE_32      5   // 32位跳转指令长度
#define NEED_SIZE_8       2   // 8位跳转指令长度

#define LEN_READ_32       4  // x86架构下，地址内容读取长度为4
#define LEN_READ_64       8  // x64架构下，地址内容读取长度为8

struct ZydisContext
{
    ZydisDecoder decoder;
    ZydisDecodedInstruction decodedInstInfo;
    ZydisDecodedOperand decodedOperandInfo[ZYDIS_MAX_OPERAND_COUNT];
    InstructionType type; // 指令类型
    bool isRelative; // 是否为相对跳转或调用指令，true表示需要重定位处理
    bool isRip;    // 是否包含RIP指令
    int ripIndex;    // rip操作数标识
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
    zyContext->isRip = IsRipInstruction(zyContext);
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
    //if (!MemoryUtils::IsMemoryReadable((BYTE*)sourceAddress, zyInstInfo.length))
    //{
    //    Logger::GetInstance().Error(L"Source instruction memory is not readable!");
    //    return false;
    //}

    bool bRes = true;

    // 在inlinehook中，涉及到的rip指令一般只包含jmp/call场景，此时可以使用常规跳转来替代rip
    // 在某些特殊场景下，如add/mov指令，此时必须按照rip处理，这种场景暂时不考虑
    switch (zyData->type)
    {
    case InstructionType::INST_JMP_NEAR:
    case InstructionType::INST_JMP_FAR:
    case InstructionType::INST_CALL_NEAR:
    case InstructionType::INST_CALL_FAR:
        bRes = RelocateRelativeJump(
                    zyData,
                    sourceAddress,
                    targetAddress,
                    bufferSize,
                    outputSize
                );
        break;
        //RelocateIndirectJump();
        break;
    case InstructionType::INST_JCC:
        //RelocateConditionalJump();
        break;
    /*
     * 间接跳转存在特殊性，它的实际流程中的需要两个跳板函数，一个前置跳板，一个常规跳板
     * 这个函数生成的是常规跳板，所以使用常规流程即可
     */
    case InstructionType::INST_JMP_INDIRECT:
    case InstructionType::INST_RET:
    case InstructionType::INST_SYSCALL:
    case InstructionType::INST_INT:
    case InstructionType::INST_NORMAL:
        CopyInstruction(
            zyData,
            sourceAddress,
            targetAddress,
            bufferSize,
            outputSize
        );
        break;
    default:
        bRes = false;
        break;
    }
    return bRes;
}

bool ZydisUtils::IsNeedFrontTrampoline(ZydisContextPtr zyData)
{
    // 只有间接寻址才需要创建前置跳板
    if (InstructionType::INST_JMP_INDIRECT != zyData->type)
    {
        return true;
    }
    return false;
}

bool ZydisUtils::GenerateFrontTrampoline(
    ZydisContextPtr zyData,
    BYTE* sourceAddress,
    BYTE* targetAddress,
    size_t* bufferSize,
    size_t* outputSize
)
{
    
    return false;
}

InstructionType ZydisUtils::GetInstructionType(const ZydisContextPtr zyContext)
{
    // 指针空判断由外部调用函数处理

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
    // 指针空判断由外部调用函数处理

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

bool ZydisUtils::IsRipInstruction(const ZydisContextPtr zyContext)
{
    // 指针空判断由外部调用函数处理

    // 检查是否为RIP相对寻址指令
    if (ZYDIS_MACHINE_MODE_LONG_64 != zyContext->decodedInstInfo.machine_mode)
    {
        return false;
    }

    for (int i = 0; i < zyContext->decodedInstInfo.operand_count; i++)
    {
        if (ZYDIS_OPERAND_TYPE_MEMORY == zyContext->decodedOperandInfo[i].type && ZYDIS_REGISTER_RIP == zyContext->decodedOperandInfo[i].mem.base)
        {
            zyContext->ripIndex = i;
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
    UINT_PTR absoluteAddr = CalculateRIPAbsoluteAddr(zyData, sourceAddress);

    if (NULL == absoluteAddr)
    {
        Logger::GetInstance().Error(L"absoluteAddr is null!");
        *outputSize = 0;
        return false;
    }

    // 计算新的偏移大小 目标地址 - 指令长度 - 存放地址
    INT64 newOffset = (INT64)absoluteAddr - (INT64)targetAddress;
    INT64 newOffset8 = newOffset - 2;
    INT64 newOffset32 = newOffset - 5;

    // 计算长度
    bool bIs64Bit = ZYDIS_MACHINE_MODE_LONG_64 == decodedInst.machine_mode;
    do
    {
        if (newOffset32 > INT32_MAX || newOffset32 < INT32_MIN)
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

        if (newOffset8 > INT8_MAX || newOffset8 < INT8_MIN)
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
    // 指令需要区分call和jmp，call存在压栈行为，需要保证栈平衡，
    // 但是call的返回值一般情况下无意义，所以不需要考虑返回值地址改变的问题
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
            if (InstructionType::INST_CALL_NEAR == zyData->type || InstructionType::INST_CALL_FAR == zyData->type)
            {
                targetAddress[11] = 0xD0; // 如果是call指令改为call
            }
            break;
        }

        if (NEED_SIZE_32 == *outputSize)
        {
            // 默认使用jmp指令跳转
            targetAddress[0] = 0xE9;
            if (InstructionType::INST_CALL_NEAR == zyData->type || InstructionType::INST_CALL_FAR == zyData->type)
            {
                targetAddress[0] = 0xE8;
            }
            memcpy(targetAddress + 1, &newOffset32, sizeof(INT32));
            break;
        }

        if (InstructionType::INST_CALL_NEAR == zyData->type || InstructionType::INST_CALL_FAR == zyData->type)
        {
            Logger::GetInstance().Error(L"rel8 call not supported!");
            return false;
        }
        targetAddress[0] = 0xEB;
        targetAddress[1] = newOffset8;
    } while (false);

    return true;
}

bool ZydisUtils::RelocateAbsoluteJump()
{
    return false;
}

bool ZydisUtils::RelocateIndirectJump()
{
    // 间接跳转的逻辑会改变常规inlinehook的流程
    // 这里的第一步是生成一个
    return false;
}

bool ZydisUtils::RelocateConditionalJump()
{
    return false;
}

bool ZydisUtils::RelocateRelativeCall()
{
    return false;
}

bool ZydisUtils::RelocateAbsoluteCall()
{
    return false;
}

bool ZydisUtils::CopyInstruction(
    ZydisContextPtr zyData,
    BYTE* sourceAddress,
    BYTE* targetAddress,
    size_t* bufferSize,
    size_t* outputSize
)
{
    if (!MemoryUtils::IsMemoryReadable(sourceAddress, zyData->decodedInstInfo.length))
    {
        Logger::GetInstance().Error(L"Source address is not readable!");
        return false;
    }

    if (!MemoryUtils::IsMemoryWritable(targetAddress, zyData->decodedInstInfo.length))
    {
        Logger::GetInstance().Error(L"Target address is not writeable!");
        return false;
    }

    memcpy(targetAddress, sourceAddress, zyData->decodedInstInfo.length);
    *outputSize = zyData->decodedInstInfo.length;
    return true;
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

UINT_PTR ZydisUtils::CalculateRIPAbsoluteAddr(
    ZydisContextPtr zyData,
    BYTE* sourceAddress
)
{
    // 绝对地址计算
    UINT_PTR absoluteAddr = NULL;
    if (zyData->isRip)
    {
        // 处理RIP相对寻址的情况：原始地址 = RIP值 + 偏移
        // RIP值 = 指令地址 + 指令长度
        const ZydisDecodedOperand* ripOp = &zyData->decodedOperandInfo[zyData->ripIndex];
        UINT_PTR ripValue = (UINT_PTR)sourceAddress + zyData->decodedInstInfo.length;
        INT64 disp = ripOp->mem.disp.value;
        UINT_PTR uAddressVal = (UINT_PTR)((INT64)ripValue + ripOp->mem.disp.value);
        memcpy(&absoluteAddr, (void*)uAddressVal, sizeof(UINT_PTR));
    }
    else
    {
        absoluteAddr = (UINT_PTR)((INT64)sourceAddress + zyData->decodedOperandInfo[0].imm.value.s + zyData->decodedInstInfo.length);
    }
    return absoluteAddr;
}

UINT64 ZydisUtils::CalculateIndirectJumpTarget(
    ZydisContextPtr zyData,
    BYTE* sourceAddress
)
{
    //ZydisDecodedOperand zyOperand = zyData->decodedOperandInfo[0];

    //// 内存间接跳转
    //if (ZYDIS_OPERAND_TYPE_MEMORY == zyOperand.type)
    //{
    //    bool bIs64Bit = ZYDIS_MACHINE_MODE_LONG_64 == zyData->decodedInstInfo.machine_mode ? true : false;
    //    int nReadLen = bIs64Bit ? LEN_READ_64 : LEN_READ_32;
    //    const ZydisDecodedOperandMem mem = zyOperand.mem;
    //    UINT64 uAddress = 0;
    //    
    //    // rip处理
    //    UINT64 uVal = 0;
    //    if (bIs64Bit && ZYDIS_REGISTER_RIP == mem.base)
    //    {
    //        uAddress = CalculateRIPAbsoluteAddr(zyData, sourceAddress);
    //        if (!ReadMemory(uAddress, nReadLen, uVal)) // 内部记录日志
    //        {
    //            return 0;
    //        }
    //        return uVal;
    //    }
    //    // jmp [rsi + rdi*4 + offset]
    //    // 基址寄存器处理
    //    if (ZYDIS_REGISTER_NONE != mem.base)
    //    {
    //        // 读取基址寄存器的值
    //        uAddress += get_register_value(mem.base);
    //    }

    //    // 比例因子处理
    //    if (ZYDIS_REGISTER_NONE != mem.index)
    //    {
    //        UINT64 uIndexVal = get_register_value(mem.index);
    //        // 架构规定，比例因子只能是1、2、4、8
    //        if (1 != mem.scale || 2 != mem.scale || 4 != mem.scale || 8 != mem.scale)
    //        {
    //            Logger::GetInstance().Error(L"Value of mem scale is invalid! value = %d", mem.scale);
    //            return 0;
    //        }
    //        uAddress += uIndexVal * mem.scale;
    //    }

    //    // 处理位移量
    //    if (mem.disp.has_displacement)
    //    {
    //        uAddress += mem.disp.value;
    //    }
    //    if (!ReadMemory(uAddress, nReadLen, uVal))
    //    {
    //        return 0;
    //    }
    //    return uVal;
    //}
    return 0;
}

bool ZydisUtils::ReadMemory(UINT_PTR ptr, UINT len, UINT64& output)
{
    output = 0;
    if (!MemoryUtils::IsMemoryReadable((BYTE*)ptr, len))
    {
        Logger::GetInstance().Error(L"Read memory failed!");
        return false;
    }

    if (LEN_READ_32 == len)
    {
        output = *(volatile uint32_t*)(ptr);
        return true;
    }
    else if (LEN_READ_64 == len)
    {
        output = *(volatile uint64_t*)(ptr);
        return true;
    }

    return false;
}
