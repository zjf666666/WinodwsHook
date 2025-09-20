#include "pch.h"
#include "ZydisUtils.h"

#include <Zydis/Zydis.h>

#include "../SecurityCore/Logger.h"
#include "../SecurityCore/MemoryUtils.h"

#define NEED_SIZE_64      12  // 64λ��תָ���
#define NEED_SIZE_32      5   // 32λ��תָ���
#define NEED_SIZE_8       2   // 8λ��תָ���

struct ZydisContext
{
    ZydisDecoder decoder;
    ZydisDecodedInstruction decodedInstInfo;
    ZydisDecodedOperand decodedOperandInfo[ZYDIS_MAX_OPERAND_COUNT];
    InstructionType type; // ָ������
    bool isRelative; // �Ƿ�Ϊ�����ת�����ָ�true��ʾ��Ҫ�ض�λ����
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
    if (nullptr == zyContext)
    {
        Logger::GetInstance().Error(L"zyContext is nullptr!");
        return false;
    }

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
    UINT_PTR absoluteAddr = (UINT_PTR)sourceAddress + decodeOperand.imm.value.s + zyData->decodedInstInfo.length;

    // �����µ�ƫ�ƴ�С Ŀ���ַ - ָ��� - ��ŵ�ַ
    INT64 newOffset = (INT32)((UINT_PTR)absoluteAddr - (UINT_PTR)targetAddress - zyData->decodedInstInfo.length);


    // ���㳤��
    bool bIs64Bit = ZYDIS_MACHINE_MODE_LONG_64 == decodedInst.machine_mode;
    do
    {
        if (newOffset > INT32_MAX || newOffset < INT32_MIN)
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

    // ��־�ں����ڲ���¼
    if (!CheckRelativeJumpParam(targetAddress, bufferSize, *outputSize))
    {
        return false;
    }

    // д��ָ��
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