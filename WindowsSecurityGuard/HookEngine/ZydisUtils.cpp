#include "pch.h"
#include "ZydisUtils.h"

#include <Zydis/Zydis.h>
#include <intrin.h>

#include "../SecurityCore/Logger.h"
#include "../SecurityCore/MemoryUtils.h"

#define NEED_SIZE_64      12  // 64λ��תָ���
#define NEED_SIZE_32      5   // 32λ��תָ���
#define NEED_SIZE_8       2   // 8λ��תָ���

#define LEN_READ_32       4  // x86�ܹ��£���ַ���ݶ�ȡ����Ϊ4
#define LEN_READ_64       8  // x64�ܹ��£���ַ���ݶ�ȡ����Ϊ8

struct ZydisContext
{
    ZydisDecoder decoder;
    ZydisDecodedInstruction decodedInstInfo;
    ZydisDecodedOperand decodedOperandInfo[ZYDIS_MAX_OPERAND_COUNT];
    InstructionType type; // ָ������
    bool isRelative; // �Ƿ�Ϊ�����ת�����ָ�true��ʾ��Ҫ�ض�λ����
    bool isRip;    // �Ƿ����RIPָ��
    int ripIndex;    // rip��������ʶ
};

// ���������ģ���ʼ��Zydis��Դ�����ز�͸��ָ��
ZydisContextPtr ZydisUtils::CreateContext(bool is64Bit)
{
    // ���Ϸ����ڲ��ṹ������ջ�ڴ����
    ZydisContext* context = (ZydisContext*)malloc(sizeof(ZydisContext));
    if (nullptr == context)
    {
        return nullptr;
    }

    // ��ʼ��������
    ZydisMachineMode zyMachineMode = ZYDIS_MACHINE_MODE_LONG_64;
    ZydisStackWidth zyStackWidth = ZYDIS_STACK_WIDTH_64;
    if (!is64Bit) // ����ϵͳ�ܹ�ѡ���Ӧ��ջ����������
    {
        zyMachineMode = ZYDIS_MACHINE_MODE_LEGACY_32;
        zyStackWidth = ZYDIS_STACK_WIDTH_32;
    }
    int nRes = ZydisDecoderInit(&context->decoder, zyMachineMode, zyStackWidth);
    if (ZYAN_STATUS_SUCCESS != nRes) // ��ʼ��ʧ�ܣ�����false
    {
        Logger::GetInstance().Error(L"ZydisDecoderInit failed! error = %d", nRes);
        return nullptr;
    }
    return context;
}

// ���������ģ��ͷ���Դ
void ZydisUtils::DestroyContext(ZydisContextPtr context)
{
    if (nullptr != context)
    {
        free(context); // �ͷŶ��ڴ�
    }
}

bool ZydisUtils::ParseInstruction(
    const void* instructionBytes,
    size_t length,
    size_t* parseLength,  // ʵ�ʽ�������
    ZydisContextPtr zyContext
)
{
    if (nullptr == zyContext)
    {
        Logger::GetInstance().Error(L"zyContext is nullptr!");
        return false;
    }

    size_t size = min(length, 20); // ������20�ֽ�
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
    // bufferSizeָ��Ϊ�գ���ʾ���㳤��
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

    // ��inlinehook�У��漰����ripָ��һ��ֻ����jmp/call��������ʱ����ʹ�ó�����ת�����rip
    // ��ĳЩ���ⳡ���£���add/movָ���ʱ���밴��rip�������ֳ�����ʱ������
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
     * �����ת���������ԣ�����ʵ�������е���Ҫ�������庯����һ��ǰ�����壬һ����������
     * ����������ɵ��ǳ������壬����ʹ�ó������̼���
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
    // ֻ�м��Ѱַ����Ҫ����ǰ������
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
    // ָ����ж����ⲿ���ú�������

    ZydisDecodedInstruction decodedInst = zyContext->decodedInstInfo;

    InstructionType type = InstructionType::INST_UNKNOWN;

    // �������Ƿ��ж�ָ������
    switch (decodedInst.mnemonic)
    {
        // ��������תָ��
    case ZYDIS_MNEMONIC_JMP:
        // �ж��Ƿ�Ϊ����ת����Ե�ַ��
        if (decodedInst.meta.branch_type == ZYDIS_BRANCH_TYPE_SHORT ||
            decodedInst.meta.branch_type == ZYDIS_BRANCH_TYPE_NEAR)
        {
            type = InstructionType::INST_JMP_NEAR;
        }
        // �ж��Ƿ�ΪԶ��ת�����Ե�ַ��
        else if (decodedInst.meta.branch_type == ZYDIS_BRANCH_TYPE_FAR)
        {
            type = InstructionType::INST_JMP_FAR;
        }
        else
        {
            type = INST_JMP_INDIRECT;
        }
        break;
        // ������תָ��
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
        // ����ָ��
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
        // ����ָ��
    case ZYDIS_MNEMONIC_RET:
    case ZYDIS_MNEMONIC_IRET:
    case ZYDIS_MNEMONIC_IRETD:
    case ZYDIS_MNEMONIC_IRETQ:
        type = INST_RET;
        break;
        // ϵͳ����ָ��
    case ZYDIS_MNEMONIC_SYSCALL:
    case ZYDIS_MNEMONIC_SYSENTER:
    case ZYDIS_MNEMONIC_SYSEXIT:
    case ZYDIS_MNEMONIC_SYSRET:
        type = INST_SYSCALL;
        break;
        // �ж�ָ��
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
    // ָ����ж����ⲿ���ú�������

    // ����Ƿ�Ϊ�����ת�����ָ��
    if ((ZYDIS_BRANCH_TYPE_SHORT == zyContext->decodedInstInfo.meta.branch_type ||
         ZYDIS_BRANCH_TYPE_NEAR == zyContext->decodedInstInfo.meta.branch_type) &&
        (ZYDIS_CATEGORY_COND_BR == zyContext->decodedInstInfo.meta.category ||
         ZYDIS_CATEGORY_UNCOND_BR == zyContext->decodedInstInfo.meta.category ||
         ZYDIS_CATEGORY_CALL == zyContext->decodedInstInfo.meta.category))
    {
        return true;
    }

    // ����Ƿ�ΪRIP���Ѱַָ��
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
    // ָ����ж����ⲿ���ú�������

    // ����Ƿ�ΪRIP���Ѱַָ��
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
    // ָ����Ч���ɵ��÷����ƣ�sourceAddress�ڴ�ɶ���Ҳ�ɵ��÷���֤
    // targetAddress����ָ��ȿ��ܷ����ı䣬��˵�ַ�ռ��Ƿ��㹻�ں����ڲ����м��
    // ���bufferSizeΪ�գ��򷵻���Ҫ�ĳ���
    ZydisDecodedInstruction decodedInst = zyData->decodedInstInfo;
    ZydisDecodedOperand decodeOperand = zyData->decodedOperandInfo[0];

    // ����ԭʼĿ���ַ ƫ�� + ָ��� + ��ǰ��ַ
    UINT_PTR absoluteAddr = CalculateRIPAbsoluteAddr(zyData, sourceAddress);

    if (NULL == absoluteAddr)
    {
        Logger::GetInstance().Error(L"absoluteAddr is null!");
        *outputSize = 0;
        return false;
    }

    // �����µ�ƫ�ƴ�С Ŀ���ַ - ָ��� - ��ŵ�ַ
    INT64 newOffset = (INT64)absoluteAddr - (INT64)targetAddress;
    INT64 newOffset8 = newOffset - 2;
    INT64 newOffset32 = newOffset - 5;

    // ���㳤��
    bool bIs64Bit = ZYDIS_MACHINE_MODE_LONG_64 == decodedInst.machine_mode;
    do
    {
        if (newOffset32 > INT32_MAX || newOffset32 < INT32_MIN)
        {
            // 64λ���Ƚ�֧��x64
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

    // ��־�ں����ڲ���¼
    if (!CheckRelativeJumpParam(targetAddress, bufferSize, *outputSize))
    {
        return false;
    }

    // д��ָ��
    // ָ����Ҫ����call��jmp��call����ѹջ��Ϊ����Ҫ��֤ջƽ�⣬
    // ����call�ķ���ֵһ������������壬���Բ���Ҫ���Ƿ���ֵ��ַ�ı������
    do
    {
        if (NEED_SIZE_64 == *outputSize)
        {
            // ����64λ��תָ��
            targetAddress[0] = 0x48;  // REX.W
            targetAddress[1] = 0xB8;  // MOV RAX, imm64
            memcpy(targetAddress + 2, &absoluteAddr, sizeof(absoluteAddr));
            targetAddress[10] = 0xFF; // JMP RAX
            targetAddress[11] = 0xE0;
            if (InstructionType::INST_CALL_NEAR == zyData->type || InstructionType::INST_CALL_FAR == zyData->type)
            {
                targetAddress[11] = 0xD0; // �����callָ���Ϊcall
            }
            break;
        }

        if (NEED_SIZE_32 == *outputSize)
        {
            // Ĭ��ʹ��jmpָ����ת
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
    // �����ת���߼���ı䳣��inlinehook������
    // ����ĵ�һ��������һ��
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
    // �����������Ƿ��㹻
    if (*bufferSize < needSize)
    {
        Logger::GetInstance().Error(L"Target address has insufficient memory!");
        return false;
    }

    // �жϿ�д�����Ƿ����
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
    // ���Ե�ַ����
    UINT_PTR absoluteAddr = NULL;
    if (zyData->isRip)
    {
        // ����RIP���Ѱַ�������ԭʼ��ַ = RIPֵ + ƫ��
        // RIPֵ = ָ���ַ + ָ���
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

    //// �ڴ�����ת
    //if (ZYDIS_OPERAND_TYPE_MEMORY == zyOperand.type)
    //{
    //    bool bIs64Bit = ZYDIS_MACHINE_MODE_LONG_64 == zyData->decodedInstInfo.machine_mode ? true : false;
    //    int nReadLen = bIs64Bit ? LEN_READ_64 : LEN_READ_32;
    //    const ZydisDecodedOperandMem mem = zyOperand.mem;
    //    UINT64 uAddress = 0;
    //    
    //    // rip����
    //    UINT64 uVal = 0;
    //    if (bIs64Bit && ZYDIS_REGISTER_RIP == mem.base)
    //    {
    //        uAddress = CalculateRIPAbsoluteAddr(zyData, sourceAddress);
    //        if (!ReadMemory(uAddress, nReadLen, uVal)) // �ڲ���¼��־
    //        {
    //            return 0;
    //        }
    //        return uVal;
    //    }
    //    // jmp [rsi + rdi*4 + offset]
    //    // ��ַ�Ĵ�������
    //    if (ZYDIS_REGISTER_NONE != mem.base)
    //    {
    //        // ��ȡ��ַ�Ĵ�����ֵ
    //        uAddress += get_register_value(mem.base);
    //    }

    //    // �������Ӵ���
    //    if (ZYDIS_REGISTER_NONE != mem.index)
    //    {
    //        UINT64 uIndexVal = get_register_value(mem.index);
    //        // �ܹ��涨����������ֻ����1��2��4��8
    //        if (1 != mem.scale || 2 != mem.scale || 4 != mem.scale || 8 != mem.scale)
    //        {
    //            Logger::GetInstance().Error(L"Value of mem scale is invalid! value = %d", mem.scale);
    //            return 0;
    //        }
    //        uAddress += uIndexVal * mem.scale;
    //    }

    //    // ����λ����
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
