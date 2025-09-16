# ParseDisplacement函数测试用例

本文档提供了一系列测试用例，用于验证ParseDisplacement函数的正确性。这些测试用例涵盖了不同的寻址模式和特殊情况，确保函数在各种情况下都能正确工作。

## 测试用例1：8位位移（mod=01）

```cpp
// 测试用例1：8位位移
// ModR/M = 01 000 001 (41h) - mod=01, reg=000, r/m=001
// 位移量 = 0x12（正数）
BYTE testCode1[] = { 0x41, 0x12 };
InstructionInfo instInfo1 = { 0 };

// 先设置ModR/M字段（通常由ParseModRM函数设置）
instInfo1.modRMFields.mod = 0x01;
instInfo1.modRMFields.reg = 0x00;
instInfo1.modRMFields.rm = 0x01;

// 调用ParseDisplacement函数
UINT bytesProcessed = ParseDisplacement(testCode1 + 1, &instInfo1);

// 验证结果
ASSERT(bytesProcessed == 1);  // 处理了1个字节
ASSERT(instInfo1.displacementSize == 1);  // 8位位移
ASSERT(instInfo1.displacement == 0x12);  // 位移值为0x12
ASSERT(instInfo1.isRelative == TRUE);  // 相对寻址
```

## 测试用例2：8位负位移（mod=01）

```cpp
// 测试用例2：8位负位移
// ModR/M = 01 000 010 (42h) - mod=01, reg=000, r/m=010
// 位移量 = 0xF0（-16，负数）
BYTE testCode2[] = { 0x42, 0xF0 };
InstructionInfo instInfo2 = { 0 };

// 先设置ModR/M字段
instInfo2.modRMFields.mod = 0x01;
instInfo2.modRMFields.reg = 0x00;
instInfo2.modRMFields.rm = 0x02;

// 调用ParseDisplacement函数
UINT bytesProcessed = ParseDisplacement(testCode2 + 1, &instInfo2);

// 验证结果
ASSERT(bytesProcessed == 1);  // 处理了1个字节
ASSERT(instInfo2.displacementSize == 1);  // 8位位移
ASSERT(instInfo2.displacement == (INT32)(INT8)0xF0);  // 位移值为-16（符号扩展）
ASSERT(instInfo2.isRelative == TRUE);  // 相对寻址
```

## 测试用例3：32位位移（mod=10）

```cpp
// 测试用例3：32位位移
// ModR/M = 10 000 010 (82h) - mod=10, reg=000, r/m=010
// 位移量 = 0x12345678
BYTE testCode3[] = { 0x82, 0x78, 0x56, 0x34, 0x12 };
InstructionInfo instInfo3 = { 0 };

// 先设置ModR/M字段
instInfo3.modRMFields.mod = 0x02;
instInfo3.modRMFields.reg = 0x00;
instInfo3.modRMFields.rm = 0x02;

// 调用ParseDisplacement函数
UINT bytesProcessed = ParseDisplacement(testCode3 + 1, &instInfo3);

// 验证结果
ASSERT(bytesProcessed == 4);  // 处理了4个字节
ASSERT(instInfo3.displacementSize == 4);  // 32位位移
ASSERT(instInfo3.displacement == 0x12345678);  // 位移值为0x12345678
ASSERT(instInfo3.isRelative == TRUE);  // 相对寻址
```

## 测试用例4：直接寻址（mod=00, r/m=101）

```cpp
// 测试用例4：直接寻址
// ModR/M = 00 000 101 (05h) - mod=00, reg=000, r/m=101
// 位移量 = 0x12345678
BYTE testCode4[] = { 0x05, 0x78, 0x56, 0x34, 0x12 };
InstructionInfo instInfo4 = { 0 };

// 先设置ModR/M字段
instInfo4.modRMFields.mod = 0x00;
instInfo4.modRMFields.reg = 0x00;
instInfo4.modRMFields.rm = 0x05;

// 调用ParseDisplacement函数
UINT bytesProcessed = ParseDisplacement(testCode4 + 1, &instInfo4);

// 验证结果
ASSERT(bytesProcessed == 4);  // 处理了4个字节
ASSERT(instInfo4.displacementSize == 4);  // 32位位移
ASSERT(instInfo4.displacement == 0x12345678);  // 位移值为0x12345678
ASSERT(instInfo4.isRelative == TRUE);  // 相对寻址
```

## 测试用例5：SIB特殊情况（mod=00, r/m=100, base=101）

```cpp
// 测试用例5：SIB特殊情况
// ModR/M = 00 000 100 (04h) - mod=00, reg=000, r/m=100
// SIB = 00 000 101 (05h) - scale=00, index=000, base=101
// 位移量 = 0x12345678
BYTE testCode5[] = { 0x04, 0x05, 0x78, 0x56, 0x34, 0x12 };
InstructionInfo instInfo5 = { 0 };

// 先设置ModR/M和SIB字段
instInfo5.modRMFields.mod = 0x00;
instInfo5.modRMFields.reg = 0x00;
instInfo5.modRMFields.rm = 0x04;
instInfo5.sibFields.scale = 0x00;
instInfo5.sibFields.index = 0x00;
instInfo5.sibFields.base = 0x05;

// 调用ParseDisplacement函数
UINT bytesProcessed = ParseDisplacement(testCode5 + 2, &instInfo5);

// 验证结果
ASSERT(bytesProcessed == 4);  // 处理了4个字节
ASSERT(instInfo5.displacementSize == 4);  // 32位位移
ASSERT(instInfo5.displacement == 0x12345678);  // 位移值为0x12345678
ASSERT(instInfo5.isRelative == TRUE);  // 相对寻址
```

## 测试用例6：无位移情况（mod=00, r/m≠101, r/m≠100）

```cpp
// 测试用例6：无位移情况
// ModR/M = 00 000 010 (02h) - mod=00, reg=000, r/m=010
BYTE testCode6[] = { 0x02 };
InstructionInfo instInfo6 = { 0 };

// 先设置ModR/M字段
instInfo6.modRMFields.mod = 0x00;
instInfo6.modRMFields.reg = 0x00;
instInfo6.modRMFields.rm = 0x02;

// 调用ParseDisplacement函数
UINT bytesProcessed = ParseDisplacement(testCode6 + 1, &instInfo6);

// 验证结果
ASSERT(bytesProcessed == 0);  // 没有处理任何字节
ASSERT(instInfo6.displacementSize == 0);  // 无位移
ASSERT(instInfo6.isRelative == FALSE);  // 不是相对寻址
```

## 测试用例7：寄存器寻址（mod=11）

```cpp
// 测试用例7：寄存器寻址
// ModR/M = 11 000 001 (C1h) - mod=11, reg=000, r/m=001
BYTE testCode7[] = { 0xC1 };
InstructionInfo instInfo7 = { 0 };

// 先设置ModR/M字段
instInfo7.modRMFields.mod = 0x03;
instInfo7.modRMFields.reg = 0x00;
instInfo7.modRMFields.rm = 0x01;

// 调用ParseDisplacement函数
UINT bytesProcessed = ParseDisplacement(testCode7 + 1, &instInfo7);

// 验证结果
ASSERT(bytesProcessed == 0);  // 没有处理任何字节
ASSERT(instInfo7.displacementSize == 0);  // 无位移
ASSERT(instInfo7.isRelative == FALSE);  // 不是相对寻址
```

## 测试用例8：无效内存访问

```cpp
// 测试用例8：无效内存访问
// ModR/M = 10 000 010 (82h) - mod=10, reg=000, r/m=010
// 但内存不可读
BYTE testCode8[] = { 0x82 };
InstructionInfo instInfo8 = { 0 };

// 先设置ModR/M字段
instInfo8.modRMFields.mod = 0x02;
instInfo8.modRMFields.reg = 0x00;
instInfo8.modRMFields.rm = 0x02;

// 假设内存不可读（只有1个字节，但需要读取4个字节）
// 调用ParseDisplacement函数
UINT bytesProcessed = ParseDisplacement(testCode8 + 1, &instInfo8);

// 验证结果
ASSERT(bytesProcessed == 0);  // 没有处理任何字节，因为内存不可读
```

## 测试执行

可以创建一个测试函数来执行所有测试用例：

```cpp
void TestParseDisplacement()
{
    // 执行测试用例1-8
    // ...
    
    printf("All ParseDisplacement tests passed!\n");
}
```

## 注意事项

1. **内存安全**：在实际测试中，需要确保测试代码中的内存是可读的。对于测试用例8，可能需要模拟内存不可读的情况。

2. **符号扩展**：特别注意测试用例2中的负位移，确保8位位移正确地符号扩展为32位。

3. **字节序**：测试代码中的位移量使用了小端字节序（x86/x64架构的标准字节序）。如果在其他架构上测试，可能需要调整字节顺序。

4. **集成测试**：这些测试用例主要针对ParseDisplacement函数的独立功能。在实际应用中，还应该进行与ParseModRM和ParseSIB函数的集成测试。

5. **边界情况**：测试用例涵盖了常见的边界情况，如无位移、寄存器寻址和无效内存访问。在实际应用中，可能还需要测试更多的边界情况。

这些测试用例可以帮助确保ParseDisplacement函数在各种情况下都能正确工作，提高代码的健壮性和可靠性。