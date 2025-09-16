#include "pch.h"
#include "InstructionRelocator.h"
#include "../SecurityCore/Logger.h"

BOOL InstructionRelocator::RelocateInstruction(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength, InstructionArchitecture arch)
{
    BOOL bRes = FALSE;
    switch (arch)
    {
    case ARCH_X86:
        bRes = RelocateInstructionX86(instInfo, newLocation, newLength);
        break;
    case ARCH_X64:
        bRes = RelocateInstructionX64(instInfo, newLocation, newLength);
        break;
    default:
        break;
    }
    return bRes;
}

BOOL InstructionRelocator::RelocateInstructionX86(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength)
{
    if (nullptr == instInfo || nullptr == newLocation || nullptr == newLength)
    {
        Logger::GetInstance().Error(L"RelocateInstruction param has nullptr!");
        return FALSE;
    }

    BOOL bRes = FALSE;
    // 根据指令类型选择不同的重定位方法
    switch (instInfo->type)
    {
    case InstructionType::INST_JMP_NEAR:
    case InstructionType::INST_JMP_SHORT:
        bRes = RelocateRelativeJump(instInfo, newLocation, newLength);
        break;
    case InstructionType::INST_CALL_NEAR:
        bRes = RelocateRelativeCall(instInfo, newLocation, newLength);
        break;
    case InstructionType::INST_COND_JMP:
        bRes = RelocateConditionalJump(instInfo, newLocation, newLength);
        break;
    case InstructionType::INST_NORMAL:
        // 普通指令直接复制
        memcpy(newLocation, instInfo->address, instInfo->length);
        *newLength = instInfo->length;
        bRes = TRUE;
        break;
    default:
        // 不支持的指令类型
        break;
    }
    return bRes;
}

BOOL InstructionRelocator::RelocateRelativeJump(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength)
{
    if (nullptr == instInfo || nullptr == newLocation || nullptr == newLength)
    {
        Logger::GetInstance().Error(L"RelocateRelativeJump param has nullptr!");
        return FALSE;
    }

    // 获取原始跳转目标地址
    UINT_PTR pOriginTargetAddr = GetOriginTargetAddress(instInfo);
    if (0 == pOriginTargetAddr)
    {
        Logger::GetInstance().Error(L"Failed to get origin target address!");
        return FALSE;
    }

    // 计算新的偏移量（从新位置到原始目标的偏移）
    INT_PTR pNewOffset = pOriginTargetAddr - ((UINT_PTR)newLocation + instInfo->length);

    // 检查偏移量是否在范围内
    if (InstructionType::INST_JMP_SHORT == instInfo->type)
    {
        // 短跳转范围检查 (-128 ~ 127)
        if (pNewOffset < -128 || pNewOffset > 127)
        {
            // 短跳转范围不足，需要转换为近跳转
            newLocation[0] = 0xE9; // 近跳转操作码
            *(INT32*)(newLocation + 1) = (INT32)pNewOffset;
            *newLength = 5; // 近跳转指令长度为5字节
        }
        else
        {
            // 保持短跳转
            newLocation[0] = 0xEB; // 短跳转操作码
            newLocation[1] = (BYTE)pNewOffset;
            *newLength = 2; // 短跳转指令长度为2字节
        }
    }
    else // INST_JMP_NEAR
    {
        // 近跳转范围检查（32位有符号整数范围）
        if (pNewOffset < INT_MIN || pNewOffset > INT_MAX)
        {
            Logger::GetInstance().Error(L"New offset out of range for near jump!");
            return FALSE;
        }

        // 创建近跳转指令
        newLocation[0] = 0xE9; // 近跳转操作码
        *(INT32*)(newLocation + 1) = (INT32)pNewOffset;
        *newLength = 5; // 近跳转指令长度为5字节
    }

    return TRUE;
}

BOOL InstructionRelocator::RelocateRelativeCall(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength)
{
    if (nullptr == instInfo || nullptr == newLocation || nullptr == newLength)
    {
        Logger::GetInstance().Error(L"RelocateRelativeCall param has nullptr!");
        return FALSE;
    }

    // 获取原始调用目标地址
    UINT_PTR pOriginTargetAddr = GetOriginTargetAddress(instInfo);
    if (0 == pOriginTargetAddr)
    {
        Logger::GetInstance().Error(L"Failed to get origin target address!");
        return FALSE;
    }

    // 计算新的偏移量（从新位置到原始目标的偏移）
    INT_PTR pNewOffset = pOriginTargetAddr - ((UINT_PTR)newLocation + 5); // CALL指令长度固定为5字节

    // 检查偏移量是否在范围内（32位有符号整数范围）
    if (pNewOffset < INT_MIN || pNewOffset > INT_MAX)
    {
        Logger::GetInstance().Error(L"New offset out of range for call instruction!");
        return FALSE;
    }

    // 创建相对调用指令
    newLocation[0] = 0xE8; // CALL操作码
    *(INT32*)(newLocation + 1) = (INT32)pNewOffset;
    *newLength = 5; // CALL指令长度为5字节

    return TRUE;
}

BOOL InstructionRelocator::RelocateConditionalJump(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength)
{
    if (nullptr == instInfo || nullptr == newLocation || nullptr == newLength)
    {
        Logger::GetInstance().Error(L"RelocateConditionalJump param has nullptr!");
        return FALSE;
    }

    // 获取原始跳转目标地址
    UINT_PTR pOriginTargetAddr = GetOriginTargetAddress(instInfo);
    if (0 == pOriginTargetAddr)
    {
        Logger::GetInstance().Error(L"Failed to get origin target address!");
        return FALSE;
    }

    // 计算新的偏移量（从新位置到原始目标的偏移）
    INT_PTR pNewOffset = pOriginTargetAddr - ((UINT_PTR)newLocation + 2); // 短条件跳转指令长度为2字节

    // 检查偏移量是否在范围内
    if (pNewOffset >= -128 && pNewOffset <= 127)
    {
        // 偏移量在短跳转范围内，直接复制原始指令并修改偏移
        newLocation[0] = instInfo->bytes[0]; // 保留原始操作码
        newLocation[1] = (BYTE)pNewOffset;   // 设置新的偏移量
        *newLength = 2; // 短条件跳转指令长度为2字节
    }
    else
    {
        // 偏移量超出短跳转范围，需要转换为近条件跳转（使用跳转链）
        // 1. 反转条件，跳转到下一条指令之后（跳过近跳转指令）
        newLocation[0] = (BYTE)(instInfo->bytes[0] ^ 0x01); // 反转条件（翻转最低位）
        newLocation[1] = 0x06; // 跳转到下一条指令之后（跳过6字节的近跳转指令）

        // 2. 添加无条件近跳转到原始目标
        newLocation[2] = 0xE9; // 近跳转操作码
        *(INT32*)(newLocation + 3) = (INT32)(pOriginTargetAddr - ((UINT_PTR)newLocation + 7)); // 计算偏移量

        *newLength = 7; // 总长度：2字节条件跳转 + 5字节无条件跳转
    }

    return TRUE;
}

UINT_PTR InstructionRelocator::GetOriginTargetAddress(const InstructionInfo* instInfo)
{
    // 不存在偏移，返回0
    if (!instInfo->displacement.isExists)
    {
        return 0;
    }

    // 计算原始目标地址（当前指令地址 + 指令长度 + 偏移量）
    UINT_PTR pOriginTargetAddr = (UINT_PTR)instInfo->address + instInfo->length + instInfo->displacement.displacement;

    return pOriginTargetAddr;
}
