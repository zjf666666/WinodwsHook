# ParseImmediate函数实现说明

## 1. 立即数概念

在x86/x64指令集中，立即数（Immediate）是指令中直接包含的常量值，用作操作数。立即数可以是8位、16位、32位或64位（仅在64位模式下），根据不同的指令和操作数前缀而变化。

特别地，操作码0xB0-0xB7表示将8位立即数移动到8位寄存器的MOV指令。这些指令在x86/x64指令集中非常常见，用于将小的常量值直接加载到特定的寄存器中。每个操作码对应不同的目标寄存器：
- 0xB0: MOV AL, imm8 (将8位立即数移动到AL寄存器)
- 0xB1: MOV CL, imm8 (将8位立即数移动到CL寄存器)
- 0xB2: MOV DL, imm8 (将8位立即数移动到DL寄存器)
- 0xB3: MOV BL, imm8 (将8位立即数移动到BL寄存器)
- 0xB4: MOV AH, imm8 (将8位立即数移动到AH寄存器)
- 0xB5: MOV CH, imm8 (将8位立即数移动到CH寄存器)
- 0xB6: MOV DH, imm8 (将8位立即数移动到DH寄存器)
- 0xB7: MOV BH, imm8 (将8位立即数移动到BH寄存器)

这些指令在Inline Hook中很重要，因为它们具有固定的指令长度（2字节：1字节操作码+1字节立即数），常用于小数据的传输，且解析相对简单。正确解析这些指令对于确保Inline Hook过程中指令重定位的准确性至关重要。

立即数主要出现在以下几种情况：

1. **算术和逻辑指令**
   - 如：`add eax, 0x12345678`（32位立即数）
   - 如：`add al, 0x12`（8位立即数）

2. **比较指令**
   - 如：`cmp ecx, 0x1000`（32位立即数）

3. **数据传送指令**
   - 如：`mov ebx, 0xFFFFFFFF`（32位立即数）
   - 如：`mov bl, 0x42`（8位立即数）

4. **跳转和调用指令**
   - 如：`push 0x12345678`（32位立即数）

5. **特殊指令**
   - 如：`int 0x80`（8位立即数，系统调用）

## 2. ParseImmediate函数流程

### 2.1 函数原型

```cpp
UINT X86InstructionParser::ParseImmediate(BYTE* codePtr, InstructionInfo* instInfo);
```

### 2.2 处理流程

1. **参数检查**
   - 验证codePtr和instInfo指针的有效性
   - 确保内存可读

2. **根据指令信息确定立即数大小**
   - 检查instInfo中的操作码和前缀信息
   - 根据指令类型和操作数前缀确定立即数大小

3. **读取立即数**
   - 根据确定的大小从codePtr指向的内存读取立即数
   - 对于小于32位的立即数，可能需要进行符号扩展

4. **更新指令信息**
   - 将立即数存储在instInfo结构体中
   - 设置immediateSize字段
   - 更新operandOffset字段
   - 设置相关标志（如果适用）

5. **返回处理的字节数**
   - 返回读取的立即数字节数

## 3. 立即数大小确定

立即数的大小取决于多个因素：

1. **指令类型**：不同指令使用不同大小的立即数
   - 例如，`mov eax, imm32`使用32位立即数
   - 而`mov al, imm8`使用8位立即数

2. **操作数大小前缀**：0x66前缀可以改变操作数大小
   - 在32位模式下，0x66前缀将32位操作数改为16位
   - 在64位模式下，0x66前缀将64位操作数改为16位

3. **REX.W前缀**：在64位模式下，REX.W=1时使用64位操作数
   - 例如，`mov rax, imm64`需要64位立即数

4. **指令编码**：某些指令有特殊的编码规则
   - 例如，`mov reg, imm`指令的立即数大小取决于寄存器大小

## 4. 代码实现

```cpp
UINT X86InstructionParser::ParseImmediate(BYTE* codePtr, InstructionInfo* instInfo)
{
    // 检查参数有效性
    if (nullptr == codePtr || nullptr == instInfo)
    {
        return 0;
    }

    // 初始化返回值（已处理的字节数）
    UINT bytesProcessed = 0;
    
    // 确定立即数大小
    UINT immediateSize = 0;
    
    // 根据指令类型和前缀确定立即数大小
    // 这里需要根据具体指令集实现详细的逻辑
    // 以下仅为示例框架
    
    BYTE opcode = instInfo->opcode.opcode;
    
    // 根据操作码确定立即数大小
    switch (opcode)
    {
        // 示例：mov reg, imm 指令
        case 0xB0: case 0xB1: case 0xB2: case 0xB3: // mov r8, imm8
        case 0xB4: case 0xB5: case 0xB6: case 0xB7:
            immediateSize = 1; // 8位立即数
            break;
            
        case 0xB8: case 0xB9: case 0xBA: case 0xBB: // mov r32, imm32
        case 0xBC: case 0xBD: case 0xBE: case 0xBF:
            // 检查操作数大小前缀
            if (instInfo->prefix.hasOperandSize) // 0x66前缀
            {
                immediateSize = 2; // 16位立即数
            }
            else
            {
                immediateSize = 4; // 32位立即数
            }
            break;
            
        // 其他指令类型...
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
            break;
            
        case 2: // 16位立即数
            instInfo->immediate = *(UINT16*)(codePtr);
            break;
            
        case 4: // 32位立即数
            instInfo->immediate = *(UINT32*)(codePtr);
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
    
    return bytesProcessed;
}
```

## 5. 注意事项

1. **架构差异**：
   - 32位和64位架构下的立即数处理有所不同
   - 64位模式下某些指令可以使用64位立即数
   - 64位模式下需要考虑REX前缀的影响

2. **字节序**：
   - x86/x64使用小端字节序
   - 多字节立即数的低字节在内存中的低地址

3. **符号扩展**：
   - 某些指令需要对小立即数进行符号扩展
   - 例如，`movsx`指令会将8位或16位立即数符号扩展为32位或64位

4. **内存安全**：
   - 在读取立即数前必须检查内存可读性
   - 防止访问无效内存导致程序崩溃

5. **与其他字段的关系**：
   - 立即数通常是指令中的最后一个字段
   - 在解析ModR/M、SIB和位移量之后解析立即数

## 6. 与指令解析流程的关系

立即数解析是指令解析流程的最后一步：

1. **解析顺序**：
   - 先解析前缀字节
   - 然后解析操作码
   - 如果需要，解析ModR/M字节
   - 如果需要，解析SIB字节
   - 如果需要，解析位移量
   - 最后解析立即数

2. **立即数位置**：
   - 立即数总是位于指令的末尾
   - 在所有其他字段之后

## 7. 测试用例

为确保ParseImmediate函数的正确性，应测试以下情况：

1. **8位立即数**
   ```cpp
   // mov al, 0x42 (B0 42)
   BYTE testCode1[] = { 0xB0, 0x42 };
   InstructionInfo instInfo1 = { 0 };
   instInfo1.opcode.opcode = 0xB0;
   UINT bytesProcessed = ParseImmediate(testCode1 + 1, &instInfo1);
   ASSERT(bytesProcessed == 1);
   ASSERT(instInfo1.immediateSize == 1);
   ASSERT(instInfo1.immediate == 0x42);
   ```

2. **16位立即数**
   ```cpp
   // mov ax, 0x1234 (66 B8 34 12)
   BYTE testCode2[] = { 0x66, 0xB8, 0x34, 0x12 };
   InstructionInfo instInfo2 = { 0 };
   instInfo2.opcode.opcode = 0xB8;
   instInfo2.prefix.hasOperandSize = TRUE;
   UINT bytesProcessed = ParseImmediate(testCode2 + 2, &instInfo2);
   ASSERT(bytesProcessed == 2);
   ASSERT(instInfo2.immediateSize == 2);
   ASSERT(instInfo2.immediate == 0x1234);
   ```

3. **32位立即数**
   ```cpp
   // mov eax, 0x12345678 (B8 78 56 34 12)
   BYTE testCode3[] = { 0xB8, 0x78, 0x56, 0x34, 0x12 };
   InstructionInfo instInfo3 = { 0 };
   instInfo3.opcode.opcode = 0xB8;
   UINT bytesProcessed = ParseImmediate(testCode3 + 1, &instInfo3);
   ASSERT(bytesProcessed == 4);
   ASSERT(instInfo3.immediateSize == 4);
   ASSERT(instInfo3.immediate == 0x12345678);
   ```

4. **64位立即数**（仅在64位模式下）
   ```cpp
   // mov rax, 0x1234567890ABCDEF (48 B8 EF CD AB 90 78 56 34 12)
   BYTE testCode4[] = { 0x48, 0xB8, 0xEF, 0xCD, 0xAB, 0x90, 0x78, 0x56, 0x34, 0x12 };
   InstructionInfo instInfo4 = { 0 };
   instInfo4.opcode.opcode = 0xB8;
   instInfo4.arch = ARCH_X64;
   instInfo4.rex.hasREXPrefix = TRUE;
   instInfo4.rex.rexW = TRUE;
   UINT bytesProcessed = ParseImmediate(testCode4 + 2, &instInfo4);
   ASSERT(bytesProcessed == 8);
   ASSERT(instInfo4.immediateSize == 8);
   ASSERT(instInfo4.immediate == 0x1234567890ABCDEF);
   ```