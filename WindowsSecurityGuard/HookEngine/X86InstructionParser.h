#pragma once
#include "IInstructionParser.h"

class X86InstructionParser : public IInstructionParser
{
public:
    // 解析单条指令，返回指令信息
    BOOL ParseInstruction(BYTE* codePtr, InstructionInfo* instInfo) override;

    // 获取指令类型
    InstructionType GetInstructionType(const InstructionInfo* instInfo) override;

    // 计算指令长度
    UINT GetInstructionLength(BYTE* codePtr) override;

    // 检查是否为相对寻址指令
    BOOL IsRelativeInstruction(const InstructionInfo* instInfo) override;

    // 获取相对寻址目标地址
    BYTE* GetRelativeTargetAddress(const InstructionInfo* instInfo) override;

private:
    /*
     * @brief 解析x86/x64指令的前缀字节
     * @param [IN] codePtr 指向当前要解析的指令字节序列的指针
     *        [IN/OUT] instInfo 指令信息结构体，用于存储解析结果
     *                 函数会更新结构体中与前缀相关的字段
     * @return UINT 解析的前缀占用的字节数，用于计算下一个解析位置
     *              如果没有前缀，返回0
     */
    UINT ParsePrefix(BYTE* codePtr, InstructionInfo* instInfo);

    // 解析操作码
    UINT ParseOpcode(BYTE* codePtr, InstructionInfo* instInfo);

    // 解析ModR/M字节
    UINT ParseModRM(BYTE* codePtr, InstructionInfo* instInfo);

    // 解析SIB字节
    UINT ParseSIB(BYTE* codePtr, InstructionInfo* instInfo);

    // 解析位移量
    UINT ParseDisplacement(BYTE* codePtr, InstructionInfo* instInfo);

    // 解析立即数
    UINT ParseImmediate(BYTE* codePtr, InstructionInfo* instInfo);
};

