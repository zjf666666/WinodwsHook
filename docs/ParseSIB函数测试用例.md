# ParseSIB函数测试用例

以下是用于测试X86InstructionParser::ParseSIB函数的测试用例。这些测试用例涵盖了SIB字节的各种组合和特殊情况，以确保函数的正确性。

## 测试用例1：基本SIB寻址（Base + Index * Scale）

```cpp
// 测试用例1：[EAX + ECX*2]
// ModR/M = 00 000 100 (04h) - mod=00, reg=000, r/m=100
// SIB = 01 001 000 (48h) - scale=01(2), index=001(ECX), base=000(EAX)
BYTE testCode1[] = { 0x04, 0x48 };
InstructionInfo instInfo1 = { 0 };

// 先设置ModR/M字段（通常由ParseModRM函数设置）
instInfo1.modRMFields.mod = 0x00;
instInfo1.modRMFields.reg = 0x00;
instInfo1.modRMFields.rm = 0x04;

// 调用ParseSIB函数
UINT bytesProcessed = ParseSIB(testCode1 + 1, &instInfo1);

// 验证结果
ASSERT(bytesProcessed == 1);  // 只处理了SIB字节
ASSERT(instInfo1.sib == 0x48);
ASSERT(instInfo1.sibFields.scale == 0x01);  // 缩放因子为2
ASSERT(instInfo1.sibFields.index == 0x01);  // ECX
ASSERT(instInfo1.sibFields.base == 0x00);   // EAX
ASSERT(instInfo1.displacementSize == 0);    // 无位移
```

## 测试用例2：不使用索引寄存器（Index=4）

```cpp
// 测试用例2：[EBX + 位移量]
// ModR/M = 01 000 100 (44h) - mod=01, reg=000, r/m=100
// SIB = 00 100 011 (23h) - scale=00(1), index=100(无索引), base=011(EBX)
// 位移量 = 0x12
BYTE testCode2[] = { 0x44, 0x23, 0x12 };
InstructionInfo instInfo2 = { 0 };

// 先设置ModR/M字段（通常由ParseModRM函数设置）
instInfo2.modRMFields.mod = 0x01;
instInfo2.modRMFields.reg = 0x00;
instInfo2.modRMFields.rm = 0x04;

// 调用ParseSIB函数
UINT bytesProcessed = ParseSIB(testCode2 + 1, &instInfo2);

// 验证结果
ASSERT(bytesProcessed == 1);  // 只处理了SIB字节，位移由ParseModRM处理
ASSERT(instInfo2.sib == 0x23);
ASSERT(instInfo2.sibFields.scale == 0x00);  // 缩放因子为1
ASSERT(instInfo2.sibFields.index == 0x04);  // 无索引寄存器
ASSERT(instInfo2.sibFields.base == 0x03);   // EBX
```

## 测试用例3：使用32位位移（Base=5, ModR/M.mod=0）

```cpp
// 测试用例3：[ESI*4 + 位移量]
// ModR/M = 00 000 100 (04h) - mod=00, reg=000, r/m=100
// SIB = 10 110 101 (B5h) - scale=10(4), index=110(ESI), base=101(特殊情况)
// 位移量 = 0x12345678
BYTE testCode3[] = { 0x04, 0xB5, 0x78, 0x56, 0x34, 0x12 };
InstructionInfo instInfo3 = { 0 };

// 先设置ModR/M字段（通常由ParseModRM函数设置）
instInfo3.modRMFields.mod = 0x00;
instInfo3.modRMFields.reg = 0x00;
instInfo3.modRMFields.rm = 0x04;

// 调用ParseSIB函数
UINT bytesProcessed = ParseSIB(testCode3 + 1, &instInfo3);

// 验证结果
ASSERT(bytesProcessed == 5);  // SIB字节 + 4字节位移
ASSERT(instInfo3.sib == 0xB5);
ASSERT(instInfo3.sibFields.scale == 0x02);  // 缩放因子为4
ASSERT(instInfo3.sibFields.index == 0x06);  // ESI
ASSERT(instInfo3.sibFields.base == 0x05);   // 特殊情况，不使用基址寄存器
ASSERT(instInfo3.displacementSize == 4);    // 32位位移
ASSERT(instInfo3.displacement == 0x12345678);
ASSERT(instInfo3.isRelative == TRUE);       // 相对寻址
```

## 测试用例4：各种缩放因子

```cpp
// 测试用例4：[EAX + EDX*8]
// ModR/M = 00 000 100 (04h) - mod=00, reg=000, r/m=100
// SIB = 11 010 000 (D0h) - scale=11(8), index=010(EDX), base=000(EAX)
BYTE testCode4[] = { 0x04, 0xD0 };
InstructionInfo instInfo4 = { 0 };

// 先设置ModR/M字段（通常由ParseModRM函数设置）
instInfo4.modRMFields.mod = 0x00;
instInfo4.modRMFields.reg = 0x00;
instInfo4.modRMFields.rm = 0x04;

// 调用ParseSIB函数
UINT bytesProcessed = ParseSIB(testCode4 + 1, &instInfo4);

// 验证结果
ASSERT(bytesProcessed == 1);  // 只处理了SIB字节
ASSERT(instInfo4.sib == 0xD0);
ASSERT(instInfo4.sibFields.scale == 0x03);  // 缩放因子为8
ASSERT(instInfo4.sibFields.index == 0x02);  // EDX
ASSERT(instInfo4.sibFields.base == 0x00);   // EAX
```

## 测试用例5：ESP作为基址寄存器

```cpp
// 测试用例5：[ESP + EBX*2 + 位移量]
// ModR/M = 01 000 100 (44h) - mod=01, reg=000, r/m=100
// SIB = 01 011 100 (5Ch) - scale=01(2), index=011(EBX), base=100(ESP)
// 位移量 = 0x12
BYTE testCode5[] = { 0x44, 0x5C, 0x12 };
InstructionInfo instInfo5 = { 0 };

// 先设置ModR/M字段（通常由ParseModRM函数设置）
instInfo5.modRMFields.mod = 0x01;
instInfo5.modRMFields.reg = 0x00;
instInfo5.modRMFields.rm = 0x04;

// 调用ParseSIB函数
UINT bytesProcessed = ParseSIB(testCode5 + 1, &instInfo5);

// 验证结果
ASSERT(bytesProcessed == 1);  // 只处理了SIB字节，位移由ParseModRM处理
ASSERT(instInfo5.sib == 0x5C);
ASSERT(instInfo5.sibFields.scale == 0x01);  // 缩放因子为2
ASSERT(instInfo5.sibFields.index == 0x03);  // EBX
ASSERT(instInfo5.sibFields.base == 0x04);   // ESP
```

## 测试用例6：EBP作为基址寄存器（mod≠0）

```cpp
// 测试用例6：[EBP + ECX*4 + 位移量]
// ModR/M = 01 000 100 (44h) - mod=01, reg=000, r/m=100
// SIB = 10 001 101 (8Dh) - scale=10(4), index=001(ECX), base=101(EBP)
// 位移量 = 0x12
BYTE testCode6[] = { 0x44, 0x8D, 0x12 };
InstructionInfo instInfo6 = { 0 };

// 先设置ModR/M字段（通常由ParseModRM函数设置）
instInfo6.modRMFields.mod = 0x01;
instInfo6.modRMFields.reg = 0x00;
instInfo6.modRMFields.rm = 0x04;

// 调用ParseSIB函数
UINT bytesProcessed = ParseSIB(testCode6 + 1, &instInfo6);

// 验证结果
ASSERT(bytesProcessed == 1);  // 只处理了SIB字节，位移由ParseModRM处理
ASSERT(instInfo6.sib == 0x8D);
ASSERT(instInfo6.sibFields.scale == 0x02);  // 缩放因子为4
ASSERT(instInfo6.sibFields.index == 0x01);  // ECX
ASSERT(instInfo6.sibFields.base == 0x05);   // EBP
```

## 测试用例7：无效内存访问

```cpp
// 测试用例7：无效内存访问
// ModR/M = 00 000 100 (04h) - mod=00, reg=000, r/m=100
// SIB = 10 110 101 (B5h) - scale=10(4), index=110(ESI), base=101(特殊情况)
// 但内存不可读
BYTE* testCode7 = nullptr;  // 无效内存
InstructionInfo instInfo7 = { 0 };

// 先设置ModR/M字段（通常由ParseModRM函数设置）
instInfo7.modRMFields.mod = 0x00;
instInfo7.modRMFields.reg = 0x00;
instInfo7.modRMFields.rm = 0x04;

// 调用ParseSIB函数
UINT bytesProcessed = ParseSIB(testCode7, &instInfo7);

// 验证结果
ASSERT(bytesProcessed == 0);  // 无效内存，返回0
```

## 测试用例8：特殊情况下的无效内存访问

```cpp
// 测试用例8：特殊情况下的无效内存访问
// ModR/M = 00 000 100 (04h) - mod=00, reg=000, r/m=100
// SIB = 10 110 101 (B5h) - scale=10(4), index=110(ESI), base=101(特殊情况)
// 但位移内存不可读
BYTE testCode8[] = { 0x04, 0xB5 };  // 缺少位移部分
InstructionInfo instInfo8 = { 0 };

// 先设置ModR/M字段（通常由ParseModRM函数设置）
instInfo8.modRMFields.mod = 0x00;
instInfo8.modRMFields.reg = 0x00;
instInfo8.modRMFields.rm = 0x04;

// 调用ParseSIB函数
UINT bytesProcessed = ParseSIB(testCode8 + 1, &instInfo8);

// 验证结果
ASSERT(bytesProcessed == 1);  // 只处理了SIB字节，位移部分无效
ASSERT(instInfo8.sib == 0xB5);
ASSERT(instInfo8.sibFields.scale == 0x02);  // 缩放因子为4
ASSERT(instInfo8.sibFields.index == 0x06);  // ESI
ASSERT(instInfo8.sibFields.base == 0x05);   // 特殊情况，不使用基址寄存器
ASSERT(instInfo8.displacementSize == 0);    // 位移无效，未设置
```

## 测试执行

可以创建一个单元测试函数来执行这些测试用例：

```cpp
void TestParseSIB()
{
    // 执行测试用例1-8
    // ...
    
    // 如果所有断言都通过，则测试成功
    printf("All ParseSIB tests passed!\n");
}
```

这些测试用例涵盖了SIB字节的各种组合和特殊情况，包括：
1. 基本SIB寻址
2. 不使用索引寄存器
3. 使用32位位移的特殊情况
4. 各种缩放因子
5. ESP作为基址寄存器
6. EBP作为基址寄存器
7. 无效内存访问
8. 特殊情况下的无效内存访问

通过这些测试用例，可以确保ParseSIB函数在各种情况下都能正确工作。