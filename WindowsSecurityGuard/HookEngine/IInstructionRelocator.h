#pragma once

#include "InstructionTypes.h"

/*
 * 重定向接口类，这部分仅用于学习该技术
 * 后续使用会选择引入已封装好的重定向器
 */
class IInstructionRelocator {
public:
    virtual ~IInstructionRelocator() {}

    // 重定位单条指令
    virtual BOOL RelocateInstruction(const InstructionInfo* instInfo, BYTE* newLocation, UINT* newLength) = 0;

    // 重定位一段代码
    virtual BOOL RelocateCode(BYTE* sourceCode, UINT minLength, BYTE* targetBuffer, UINT* relocatedLength) = 0;

    // 创建跳转指令（从新位置跳回原位置）
    virtual BOOL CreateJumpBack(BYTE* sourceLocation, BYTE* targetLocation, BYTE* buffer, UINT* length) = 0;
};

