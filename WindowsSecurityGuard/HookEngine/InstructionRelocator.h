#pragma once

#include <Windows.h>

#include "InstructionTypes.h"

class InstructionRelocator
{
public:
    // 重定位单条指令
    static BOOL RelocateInstruction(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength, InstructionArchitecture arch);

private:
    static BOOL RelocateInstructionX86(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength);
    
    static BOOL RelocateInstructionX64(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength);

private:
    // 重定位相对跳转指令
    static BOOL RelocateRelativeJump(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength);

    // 重定位相对调用指令
    static BOOL RelocateRelativeCall(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength);

    // 重定位条件跳转指令
    static BOOL RelocateConditionalJump(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength);

    // 创建绝对跳转指令
    static BOOL CreateAbsoluteJump(BYTE* targetAddress, BYTE* buffer, UINT* length);

    // 获取原来的指令指向地址
    static UINT_PTR GetOriginTargetAddress(const InstructionInfo* instInfo);
};

