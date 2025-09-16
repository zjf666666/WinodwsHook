# ParseImmediate函数实现代码

本文档提供了`X86InstructionParser::ParseImmediate`函数的完整实现代码，该函数用于解析x86/x64指令中的立即数字段。

## 函数实现

```cpp
UINT X86InstructionParser::ParseImmediate(BYTE* codePtr, InstructionInfo* instInfo)
{
    // 参数检查
    if (nullptr == codePtr || nullptr == instInfo)
    {
        return 0;
    }

    // 初始化返回值（已处理的字节数）
    UINT bytesProcessed = 0;
    
    // 确定立即数大小
    UINT immediateSize = 0;
    
    // 根据指令类型确定立即数大小
    switch (instInfo->type)
    {
        // 立即数到寄存器的数据传送指令
        case INST_MOV_IMM_TO_REG:
            // 检查操作码范围
            if (instInfo->opcode.opcode >= 0xB0 && instInfo->opcode.opcode <= 0xB7)
            {
                // MOV r8, imm8 (B0-B7)
                immediateSize = 1;
            }
            else if (instInfo->opcode.opcode >= 0xB8 && instInfo->opcode.opcode <= 0xBF)
            {
                // MOV r16/r32/r64, imm16/imm32/imm64 (B8-BF)
                if (instInfo->prefix.hasOperandSize) // 0x66前缀
                {
                    immediateSize = 2; // 16位立即数
                }
                else if (instInfo->arch == ARCH_X64 && instInfo->rex.hasREXPrefix && instInfo->rex.rexW)
                {
                    immediateSize = 8; // 64位立即数（仅在64位模式下且REX.W=1）
                }
                else
                {
                    immediateSize = 4; // 32位立即数
                }
            }
            break;
            
        // 立即数到寄存器/内存的数据传送指令
        case INST_MOV_IMM_TO_RM:
            // MOV r/m8, imm8 (C6)
            if (instInfo->opcode.opcode == 0xC6)
            {
                immediateSize = 1;
            }
            // MOV r/m16/r/m32/r/m64, imm16/imm32 (C7)
            else if (instInfo->opcode.opcode == 0xC7)
            {
                if (instInfo->prefix.hasOperandSize) // 0x66前缀
                {
                    immediateSize = 2; // 16位立即数
                }
                else
                {
                    immediateSize = 4; // 32位立即数（即使在64位模式下也是32位立即数）
                }
            }
            break;
            
        // 立即数算术和逻辑指令
        case INST_ARITHMETIC_IMM:
            // 检查ModR/M.reg字段（扩展操作码）
            BYTE extendedOpcode = instInfo->modRM.reg;
            
            // 8位立即数指令 (80, 82, 83)
            if (instInfo->opcode.opcode == 0x80 || instInfo->opcode.opcode == 0x82)
            {
                immediateSize = 1; // 8位立即数
            }
            // 16/32位立即数指令 (81)
            else if (instInfo->opcode.opcode == 0x81)
            {
                if (instInfo->prefix.hasOperandSize) // 0x66前缀
                {
                    immediateSize = 2; // 16位立即数
                }
                else
                {
                    immediateSize = 4; // 32位立即数
                }
            }
            // 带符号扩展的8位立即数指令 (83)
            else if (instInfo->opcode.opcode == 0x83)
            {
                immediateSize = 1; // 8位立即数（会被符号扩展）
            }
            break;
            
        // 比较立即数指令
        case INST_CMP_IMM:
            // 与算术指令类似的处理逻辑
            if (instInfo->opcode.opcode == 0x3C) // CMP AL, imm8
            {
                immediateSize = 1;
            }
            else if (instInfo->opcode.opcode == 0x3D) // CMP AX/EAX/RAX, imm16/imm32
            {
                if (instInfo->prefix.hasOperandSize) // 0x66前缀
                {
                    immediateSize = 2; // 16位立即数
                }
                else
                {
                    immediateSize = 4; // 32位立即数
                }
            }
            break;
            
        // 测试立即数指令
        case INST_TEST_IMM:
            // 与比较指令类似的处理逻辑
            if (instInfo->opcode.opcode == 0xA8) // TEST AL, imm8
            {
                immediateSize = 1;
            }
            else if (instInfo->opcode.opcode == 0xA9) // TEST AX/EAX/RAX, imm16/imm32
            {
                if (instInfo->prefix.hasOperandSize) // 0x66前缀
                {
                    immediateSize = 2; // 16位立即数
                }
                else
                {
                    immediateSize = 4; // 32位立即数
                }
            }
            break;
            
        // 相对跳转指令
        case INST_JMP_REL:
            if (instInfo->opcode.opcode == 0xEB) // JMP rel8
            {
                immediateSize = 1;
            }
            else if (instInfo->opcode.opcode == 0xE9) // JMP rel16/rel32
            {
                if (instInfo->prefix.hasOperandSize) // 0x66前缀
                {
                    immediateSize = 2; // 16位相对偏移
                }
                else
                {
                    immediateSize = 4; // 32位相对偏移
                }
            }
            break;
            
        // 条件跳转指令
        case INST_JCC:
            if ((instInfo->opcode.opcode >= 0x70 && instInfo->opcode.opcode <= 0x7F) ||
                instInfo->opcode.opcode == 0xE3) // Jcc rel8, JCXZ/JECXZ/JRCXZ rel8
            {
                immediateSize = 1; // 8位相对偏移
            }
            else if (instInfo->opcode.opcode2 >= 0x80 && instInfo->opcode.opcode2 <= 0x8F) // Jcc rel16/rel32
            {
                if (instInfo->prefix.hasOperandSize) // 0x66前缀
                {
                    immediateSize = 2; // 16位相对偏移
                }
                else
                {
                    immediateSize = 4; // 32位相对偏移
                }
            }
            break;
            
        // 调用指令
        case INST_CALL:
            if (instInfo->opcode.opcode == 0xE8) // CALL rel16/rel32
            {
                if (instInfo->prefix.hasOperandSize) // 0x66前缀
                {
                    immediateSize = 2; // 16位相对偏移
                }
                else
                {
                    immediateSize = 4; // 32位相对偏移
                }
            }
            break;
            
        // 立即数压栈指令
        case INST_PUSH_IMM:
            if (instInfo->opcode.opcode == 0x6A) // PUSH imm8
            {
                immediateSize = 1; // 8位立即数（会被符号扩展）
            }
            else if (instInfo->opcode.opcode == 0x68) // PUSH imm16/imm32
            {
                if (instInfo->prefix.hasOperandSize) // 0x66前缀
                {
                    immediateSize = 2; // 16位立即数
                }
                else
                {
                    immediateSize = 4; // 32位立即数
                }
            }
            break;
            
        // 中断指令
        case INST_INT:
            if (instInfo->opcode.opcode == 0xCD) // INT imm8
            {
                immediateSize = 1; // 8位立即数
            }
            break;
            
        // 其他可能包含立即数的指令类型...
        // 根据需要添加更多指令类型的处理
    }
    
    // 如果没有立即数，直接返回
    if (immediateSize == 0)
    {
        return 0;
    }
    
    // 检查内存是否可读
    if (!MemoryUtils::IsMemoryReadable(codePtr, immediateSize))
    {
        return 0;
    }
    
    // 读取立即数
    switch (immediateSize)
    {
        case 1: // 8位立即数
            instInfo->immediate = *(UINT8*)(codePtr);
            // 对于某些指令，需要进行符号扩展
            if (NeedsSignExtension(instInfo->type, instInfo->opcode.opcode))
            {
                // 符号扩展8位立即数到32位或64位
                if ((*(UINT8*)(codePtr)) & 0x80) // 检查符号位
                {
                    // 负数，扩展1
                    instInfo->immediate |= 0xFFFFFF00;
                    if (instInfo->arch == ARCH_X64)
                    {
                        instInfo->immediate |= 0xFFFFFFFF00000000;
                    }
                }
            }
            break;
            
        case 2: // 16位立即数
            instInfo->immediate = *(UINT16*)(codePtr);
            // 对于某些指令，需要进行符号扩展
            if (NeedsSignExtension(instInfo->type, instInfo->opcode.opcode))
            {
                // 符号扩展16位立即数到32位或64位
                if ((*(UINT16*)(codePtr)) & 0x8000) // 检查符号位
                {
                    // 负数，扩展1
                    instInfo->immediate |= 0xFFFF0000;
                    if (instInfo->arch == ARCH_X64)
                    {
                        instInfo->immediate |= 0xFFFFFFFF00000000;
                    }
                }
            }
            break;
            
        case 4: // 32位立即数
            instInfo->immediate = *(UINT32*)(codePtr);
            // 在64位模式下，对于某些指令，需要进行符号扩展
            if (instInfo->arch == ARCH_X64 && NeedsSignExtension(instInfo->type, instInfo->opcode.opcode))
            {
                // 符号扩展32位立即数到64位
                if ((*(UINT32*)(codePtr)) & 0x80000000) // 检查符号位
                {
                    // 负数，扩展1
                    instInfo->immediate |= 0xFFFFFFFF00000000;
                }
            }
            break;
            
        case 8: // 64位立即数（仅在64位模式下）
            if (instInfo->arch == ARCH_X64)
            {
                instInfo->immediate = *(UINT64*)(codePtr);
            }
            break;
    }
    
    // 更新指令信息
    instInfo->immediateSize = immediateSize;
    bytesProcessed = immediateSize;
    
    // 设置操作数偏移量
    instInfo->operandOffset = bytesProcessed - immediateSize;
    
    // 对于相对跳转指令，设置isRelative标志
    if (IsRelativeJumpOrCall(instInfo->type))
    {
        instInfo->isRelative = TRUE;
        
        // 计算目标地址
        UINT64 nextInstructionAddress = instInfo->address + instInfo->length;
        
        // 根据立即数大小计算相对偏移
        switch (immediateSize)
        {
            case 1: // 8位相对偏移
                // 需要符号扩展
                if (instInfo->immediate & 0x80) // 负偏移
                {
                    instInfo->relativeTarget = nextInstructionAddress - ((~instInfo->immediate & 0xFF) + 1);
                }
                else // 正偏移
                {
                    instInfo->relativeTarget = nextInstructionAddress + instInfo->immediate;
                }
                break;
                
            case 2: // 16位相对偏移
                // 需要符号扩展
                if (instInfo->immediate & 0x8000) // 负偏移
                {
                    instInfo->relativeTarget = nextInstructionAddress - ((~instInfo->immediate & 0xFFFF) + 1);
                }
                else // 正偏移
                {
                    instInfo->relativeTarget = nextInstructionAddress + instInfo->immediate;
                }
                break;
                
            case 4: // 32位相对偏移
                // 需要符号扩展
                if (instInfo->immediate & 0x80000000) // 负偏移
                {
                    instInfo->relativeTarget = nextInstructionAddress - ((~instInfo->immediate & 0xFFFFFFFF) + 1);
                }
                else // 正偏移
                {
                    instInfo->relativeTarget = nextInstructionAddress + instInfo->immediate;
                }
                break;
        }
    }
    
    return bytesProcessed;
}

// 辅助函数：判断指令是否需要对立即数进行符号扩展
BOOL X86InstructionParser::NeedsSignExtension(InstructionType type, BYTE opcode)
{
    // 以下指令类型需要对立即数进行符号扩展
    switch (type)
    {
        case INST_JMP_REL:      // 相对跳转指令
        case INST_JCC:          // 条件跳转指令
        case INST_CALL:         // 调用指令
            return TRUE;
            
        case INST_ARITHMETIC_IMM:
            // 对于0x83操作码（带符号扩展的8位立即数），需要符号扩展
            if (opcode == 0x83)
            {
                return TRUE;
            }
            break;
            
        case INST_PUSH_IMM:
            // PUSH imm8 (0x6A) 需要符号扩展
            if (opcode == 0x6A)
            {
                return TRUE;
            }
            break;
    }
    
    return FALSE;
}

// 辅助函数：判断指令是否为相对跳转或调用指令
BOOL X86InstructionParser::IsRelativeJumpOrCall(InstructionType type)
{
    return (type == INST_JMP_REL || type == INST_JCC || type == INST_CALL);
}
```

## 函数说明

1. **参数检查**：
   - 验证`codePtr`和`instInfo`指针的有效性
   - 确保内存可读

2. **立即数大小确定**：
   - 根据指令类型（`instInfo->type`）确定立即数大小
   - 考虑操作码、ModR/M.reg（扩展操作码）、前缀等因素
   - 支持8位、16位、32位和64位（仅在64位模式下）立即数

3. **立即数读取**：
   - 根据确定的大小从`codePtr`指向的内存读取立即数
   - 对于需要符号扩展的指令，进行适当的符号扩展处理

4. **指令信息更新**：
   - 设置`immediateSize`字段
   - 更新`operandOffset`字段
   - 对于相对跳转指令，设置`isRelative`标志并计算目标地址

5. **辅助函数**：
   - `NeedsSignExtension`：判断指令是否需要对立即数进行符号扩展
   - `IsRelativeJumpOrCall`：判断指令是否为相对跳转或调用指令

## 注意事项

1. **内存安全**：
   - 在读取立即数前必须检查内存可读性
   - 使用`MemoryUtils::IsMemoryReadable`函数确保内存访问安全

2. **错误处理**：
   - 对于无效参数或内存不可读的情况，返回0表示未处理任何字节
   - 对于不包含立即数的指令，也返回0

3. **符号扩展**：
   - 对于某些指令（如相对跳转、带符号扩展的算术指令等），需要对立即数进行符号扩展
   - 符号扩展的处理取决于立即数大小和指令类型

4. **相对地址计算**：
   - 对于相对跳转和调用指令，需要计算目标地址
   - 目标地址 = 下一条指令地址 + 相对偏移
   - 相对偏移可能是正数或负数，需要正确处理符号位

5. **架构差异**：
   - 32位和64位架构下的立即数处理有所不同
   - 64位模式下需要考虑REX前缀的影响
   - 64位模式下某些指令可以使用64位立即数

6. **指令类型识别**：
   - 函数依赖于之前的指令类型识别（`instInfo->type`）
   - 确保在调用`ParseImmediate`前已正确设置指令类型

7. **字节序**：
   - x86/x64使用小端字节序
   - 多字节立即数的低字节在内存中的低地址