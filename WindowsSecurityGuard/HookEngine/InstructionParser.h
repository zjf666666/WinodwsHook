#pragma once

#include <Windows.h>

#include "InstructionTypes.h"

class InstructionParser
{
public:
    static BOOL ParseInstruction(BYTE* codePtr, InstructionInfo* instInfo, InstructionArchitecture arch);

private:
    static BOOL ParseInstructionX86(BYTE* codePtr, InstructionInfo* instInfo);

    static BOOL ParseInstructionX64(BYTE* codePtr, InstructionInfo* instInfo);

private:
    // 获取指令类型
    static InstructionType GetInstructionType(const InstructionInfo* instInfo);

    // 检查是否为相对寻址指令
    static BOOL IsRelativeInstruction(const InstructionInfo* instInfo);

    /*
     * @brief 解析x86/x64指令的前缀字节
     * @param [IN] codePtr 指向当前要解析的指令字节序列的指针
     *        [IN/OUT] instInfo 指令信息结构体，用于存储解析结果
     *                 函数会更新结构体中与前缀相关的字段
     * @return UINT 解析的前缀占用的字节数，用于计算下一个解析位置
     *              如果没有前缀，返回0
     */
    static UINT ParsePrefix(BYTE* codePtr, InstructionInfo* instInfo);

    // 解析操作码
    static UINT ParseOpcode(BYTE* codePtr, InstructionInfo* instInfo);

    // 解析ModR/M字节
    static UINT ParseModRM(BYTE* codePtr, InstructionInfo* instInfo);

    // 解析SIB字节
    static UINT ParseSIB(BYTE* codePtr, InstructionInfo* instInfo);

    // 解析位移量
    static UINT ParseDisplacement(BYTE* codePtr, InstructionInfo* instInfo);

    // 解析立即数
    static UINT ParseImmediate(BYTE* codePtr, InstructionInfo* instInfo);

    // 获取位移量大小
    static UINT GetDisplacementSize(InstructionInfo* instInfo);
};

