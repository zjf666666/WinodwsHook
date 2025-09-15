#pragma once

#include "InstructionTypes.h"

/* 
 * 指令解析器接口类，这部分仅用于学习该技术
 * 后续使用会选择引入已封装好的解析器
 */
class IInstructionParser
{
public:
    virtual ~IInstructionParser() {}

    /*
     * @brief 解析指定内存地址处的单条CPU指令，提取其结构信息
     * @param [IN] codePtr 指向要解析的指令字节序列的指针
     *        [OUT] instInfo 用于存储解析结果的指令信息结构体指针
     * @return BOOL 成功解析返回TRUE，失败返回FALSE
     */
    virtual BOOL ParseInstruction(BYTE* codePtr, InstructionInfo* instInfo) = 0;

    // 获取指令类型
    virtual InstructionType GetInstructionType(const InstructionInfo* instInfo) = 0;

    // 计算指令长度
    virtual UINT GetInstructionLength(BYTE* codePtr) = 0;

    // 检查是否为相对寻址指令
    virtual BOOL IsRelativeInstruction(const InstructionInfo* instInfo) = 0;

    // 获取相对寻址目标地址
    virtual BYTE* GetRelativeTargetAddress(const InstructionInfo* instInfo) = 0;
};