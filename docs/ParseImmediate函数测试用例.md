# ParseImmediate函数测试用例

本文档提供了`X86InstructionParser::ParseImmediate`函数的测试用例，用于验证该函数在各种情况下的正确性。

## 测试用例概述

测试用例涵盖了以下几个方面：

1. 不同大小的立即数（8位、16位、32位、64位）
2. 不同类型的指令（数据传送、算术、跳转等）
3. 前缀对立即数大小的影响
4. 符号扩展的处理
5. 相对跳转指令的目标地址计算
6. 错误处理和边界情况

## 测试用例1：8位立即数指令

```cpp
TEST(X86InstructionParserTest, ParseImmediate_8BitImmediate)
{
    // 测试用例：mov al, 0x42 (B0 42)
    BYTE testCode[] = { 0xB0, 0x42 };
    
    // 初始化指令信息
    InstructionInfo instInfo = { 0 };
    instInfo.type = INST_MOV_IMM_TO_REG;
    instInfo.opcode.opcode = 0xB0;
    instInfo.address = (UINT64)testCode;
    instInfo.length = 2;
    
    // 调用ParseImmediate函数
    X86InstructionParser parser;
    UINT bytesProcessed = parser.ParseImmediate(testCode + 1, &instInfo);
    
    // 验证结果
    EXPECT_EQ(bytesProcessed, 1);
    EXPECT_EQ(instInfo.immediateSize, 1);
    EXPECT_EQ(instInfo.immediate, 0x42);
    EXPECT_EQ(instInfo.operandOffset, 0);
    EXPECT_FALSE(instInfo.isRelative);
}
```

## 测试用例2：16位立即数指令（带操作数大小前缀）

```cpp
TEST(X86InstructionParserTest, ParseImmediate_16BitImmediate)
{
    // 测试用例：mov ax, 0x1234 (66 B8 34 12)
    BYTE testCode[] = { 0x66, 0xB8, 0x34, 0x12 };
    
    // 初始化指令信息
    InstructionInfo instInfo = { 0 };
    instInfo.type = INST_MOV_IMM_TO_REG;
    instInfo.opcode.opcode = 0xB8;
    instInfo.prefix.hasOperandSize = TRUE;
    instInfo.address = (UINT64)testCode;
    instInfo.length = 4;
    
    // 调用ParseImmediate函数
    X86InstructionParser parser;
    UINT bytesProcessed = parser.ParseImmediate(testCode + 2, &instInfo);
    
    // 验证结果
    EXPECT_EQ(bytesProcessed, 2);
    EXPECT_EQ(instInfo.immediateSize, 2);
    EXPECT_EQ(instInfo.immediate, 0x1234);
    EXPECT_EQ(instInfo.operandOffset, 0);
    EXPECT_FALSE(instInfo.isRelative);
}
```

## 测试用例3：32位立即数指令

```cpp
TEST(X86InstructionParserTest, ParseImmediate_32BitImmediate)
{
    // 测试用例：mov eax, 0x12345678 (B8 78 56 34 12)
    BYTE testCode[] = { 0xB8, 0x78, 0x56, 0x34, 0x12 };
    
    // 初始化指令信息
    InstructionInfo instInfo = { 0 };
    instInfo.type = INST_MOV_IMM_TO_REG;
    instInfo.opcode.opcode = 0xB8;
    instInfo.address = (UINT64)testCode;
    instInfo.length = 5;
    
    // 调用ParseImmediate函数
    X86InstructionParser parser;
    UINT bytesProcessed = parser.ParseImmediate(testCode + 1, &instInfo);
    
    // 验证结果
    EXPECT_EQ(bytesProcessed, 4);
    EXPECT_EQ(instInfo.immediateSize, 4);
    EXPECT_EQ(instInfo.immediate, 0x12345678);
    EXPECT_EQ(instInfo.operandOffset, 0);
    EXPECT_FALSE(instInfo.isRelative);
}
```

## 测试用例4：64位立即数指令（仅在64位模式下）

```cpp
TEST(X86InstructionParserTest, ParseImmediate_64BitImmediate)
{
    // 测试用例：mov rax, 0x1234567890ABCDEF (48 B8 EF CD AB 90 78 56 34 12)
    BYTE testCode[] = { 0x48, 0xB8, 0xEF, 0xCD, 0xAB, 0x90, 0x78, 0x56, 0x34, 0x12 };
    
    // 初始化指令信息
    InstructionInfo instInfo = { 0 };
    instInfo.type = INST_MOV_IMM_TO_REG;
    instInfo.opcode.opcode = 0xB8;
    instInfo.arch = ARCH_X64;
    instInfo.rex.hasREXPrefix = TRUE;
    instInfo.rex.rexW = TRUE;
    instInfo.address = (UINT64)testCode;
    instInfo.length = 10;
    
    // 调用ParseImmediate函数
    X86InstructionParser parser;
    UINT bytesProcessed = parser.ParseImmediate(testCode + 2, &instInfo);
    
    // 验证结果
    EXPECT_EQ(bytesProcessed, 8);
    EXPECT_EQ(instInfo.immediateSize, 8);
    EXPECT_EQ(instInfo.immediate, 0x1234567890ABCDEF);
    EXPECT_EQ(instInfo.operandOffset, 0);
    EXPECT_FALSE(instInfo.isRelative);
}
```

## 测试用例5：带符号扩展的8位立即数指令

```cpp
TEST(X86InstructionParserTest, ParseImmediate_8BitSignExtended)
{
    // 测试用例：add eax, 0xF0 (83 C0 F0) - 带符号扩展的8位立即数
    BYTE testCode[] = { 0x83, 0xC0, 0xF0 };
    
    // 初始化指令信息
    InstructionInfo instInfo = { 0 };
    instInfo.type = INST_ARITHMETIC_IMM;
    instInfo.opcode.opcode = 0x83;
    instInfo.modRM.reg = 0; // ADD操作
    instInfo.address = (UINT64)testCode;
    instInfo.length = 3;
    
    // 调用ParseImmediate函数
    X86InstructionParser parser;
    UINT bytesProcessed = parser.ParseImmediate(testCode + 2, &instInfo);
    
    // 验证结果
    EXPECT_EQ(bytesProcessed, 1);
    EXPECT_EQ(instInfo.immediateSize, 1);
    
    // 0xF0应该被符号扩展为0xFFFFFFF0（-16的补码表示）
    EXPECT_EQ(instInfo.immediate & 0xFFFFFFFF, 0xFFFFFFF0);
    EXPECT_EQ(instInfo.operandOffset, 0);
    EXPECT_FALSE(instInfo.isRelative);
}
```

## 测试用例6：相对跳转指令（8位偏移）

```cpp
TEST(X86InstructionParserTest, ParseImmediate_RelativeJump8)
{
    // 测试用例：jmp short 0x05 (EB 05) - 向前跳转5字节
    BYTE testCode[] = { 0xEB, 0x05 };
    
    // 初始化指令信息
    InstructionInfo instInfo = { 0 };
    instInfo.type = INST_JMP_REL;
    instInfo.opcode.opcode = 0xEB;
    instInfo.address = (UINT64)testCode;
    instInfo.length = 2;
    
    // 调用ParseImmediate函数
    X86InstructionParser parser;
    UINT bytesProcessed = parser.ParseImmediate(testCode + 1, &instInfo);
    
    // 验证结果
    EXPECT_EQ(bytesProcessed, 1);
    EXPECT_EQ(instInfo.immediateSize, 1);
    EXPECT_EQ(instInfo.immediate, 0x05);
    EXPECT_EQ(instInfo.operandOffset, 0);
    EXPECT_TRUE(instInfo.isRelative);
    
    // 验证目标地址计算
    // 目标地址 = 下一条指令地址 + 相对偏移 = (testCode + 2) + 5 = testCode + 7
    EXPECT_EQ(instInfo.relativeTarget, (UINT64)(testCode + 7));
}
```

## 测试用例7：相对跳转指令（负偏移）

```cpp
TEST(X86InstructionParserTest, ParseImmediate_RelativeJumpNegative)
{
    // 测试用例：jmp short 0xFB (EB FB) - 向后跳转5字节
    BYTE testCode[] = { 0xEB, 0xFB };
    
    // 初始化指令信息
    InstructionInfo instInfo = { 0 };
    instInfo.type = INST_JMP_REL;
    instInfo.opcode.opcode = 0xEB;
    instInfo.address = (UINT64)testCode;
    instInfo.length = 2;
    
    // 调用ParseImmediate函数
    X86InstructionParser parser;
    UINT bytesProcessed = parser.ParseImmediate(testCode + 1, &instInfo);
    
    // 验证结果
    EXPECT_EQ(bytesProcessed, 1);
    EXPECT_EQ(instInfo.immediateSize, 1);
    EXPECT_EQ(instInfo.immediate, 0xFB);
    EXPECT_EQ(instInfo.operandOffset, 0);
    EXPECT_TRUE(instInfo.isRelative);
    
    // 验证目标地址计算
    // 0xFB的补码表示为-5
    // 目标地址 = 下一条指令地址 + 相对偏移 = (testCode + 2) - 5 = testCode - 3
    EXPECT_EQ(instInfo.relativeTarget, (UINT64)(testCode - 3));
}
```

## 测试用例8：32位相对调用指令

```cpp
TEST(X86InstructionParserTest, ParseImmediate_RelativeCall32)
{
    // 测试用例：call 0x12345678 (E8 78 56 34 12) - 相对调用
    BYTE testCode[] = { 0xE8, 0x78, 0x56, 0x34, 0x12 };
    
    // 初始化指令信息
    InstructionInfo instInfo = { 0 };
    instInfo.type = INST_CALL;
    instInfo.opcode.opcode = 0xE8;
    instInfo.address = (UINT64)testCode;
    instInfo.length = 5;
    
    // 调用ParseImmediate函数
    X86InstructionParser parser;
    UINT bytesProcessed = parser.ParseImmediate(testCode + 1, &instInfo);
    
    // 验证结果
    EXPECT_EQ(bytesProcessed, 4);
    EXPECT_EQ(instInfo.immediateSize, 4);
    EXPECT_EQ(instInfo.immediate, 0x12345678);
    EXPECT_EQ(instInfo.operandOffset, 0);
    EXPECT_TRUE(instInfo.isRelative);
    
    // 验证目标地址计算
    // 目标地址 = 下一条指令地址 + 相对偏移 = (testCode + 5) + 0x12345678
    EXPECT_EQ(instInfo.relativeTarget, (UINT64)(testCode + 5) + 0x12345678);
}
```

## 测试用例9：空指针和无效参数处理

```cpp
TEST(X86InstructionParserTest, ParseImmediate_NullPointers)
{
    // 测试用例：传入空指针
    BYTE testCode[] = { 0xB8, 0x78, 0x56, 0x34, 0x12 };
    InstructionInfo instInfo = { 0 };
    X86InstructionParser parser;
    
    // 测试codePtr为空指针
    UINT bytesProcessed = parser.ParseImmediate(nullptr, &instInfo);
    EXPECT_EQ(bytesProcessed, 0);
    
    // 测试instInfo为空指针
    bytesProcessed = parser.ParseImmediate(testCode, nullptr);
    EXPECT_EQ(bytesProcessed, 0);
}
```

## 测试用例10：不包含立即数的指令

```cpp
TEST(X86InstructionParserTest, ParseImmediate_NoImmediate)
{
    // 测试用例：mov eax, ecx (89 C8) - 不包含立即数
    BYTE testCode[] = { 0x89, 0xC8 };
    
    // 初始化指令信息
    InstructionInfo instInfo = { 0 };
    instInfo.type = INST_MOV_REG_TO_REG;
    instInfo.opcode.opcode = 0x89;
    instInfo.address = (UINT64)testCode;
    instInfo.length = 2;
    
    // 调用ParseImmediate函数
    X86InstructionParser parser;
    UINT bytesProcessed = parser.ParseImmediate(testCode + 2, &instInfo);
    
    // 验证结果 - 应该返回0，表示没有处理任何字节
    EXPECT_EQ(bytesProcessed, 0);
    EXPECT_EQ(instInfo.immediateSize, 0);
}
```

## 测试执行方法

以下是执行测试的示例代码：

```cpp
// 在main函数或测试入口点调用
int main(int argc, char** argv)
{
    // 初始化Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // 运行所有测试
    return RUN_ALL_TESTS();
}
```

## 测试注意事项

1. **内存安全**：
   - 测试用例应确保内存访问安全
   - 避免访问无效内存或越界访问

2. **架构差异**：
   - 某些测试用例（如64位立即数测试）仅适用于特定架构
   - 在不同架构下运行测试时应考虑这一点

3. **指令类型依赖**：
   - `ParseImmediate`函数依赖于正确的指令类型识别
   - 测试用例应确保正确设置`instInfo.type`字段

4. **前缀影响**：
   - 操作数大小前缀（0x66）和REX前缀会影响立即数大小
   - 测试用例应覆盖这些情况

5. **相对地址计算**：
   - 对于相对跳转和调用指令，应验证目标地址计算的正确性
   - 考虑正偏移和负偏移的情况

6. **符号扩展**：
   - 对于需要符号扩展的指令，应验证符号扩展的正确性
   - 测试正数和负数的情况