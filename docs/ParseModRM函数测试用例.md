# ParseModRM函数测试用例

为了验证ParseModRM函数的正确性，以下是一些测试用例，涵盖了不同的ModR/M字节组合和寻址模式。

## 测试用例1：寄存器寻址（mod=11）

```cpp
// 测试ModR/M字节：11 000 001 (0xC1) - mod=11, reg=000, rm=001
// 表示寄存器到寄存器操作，如：add ecx, eax
BYTE testCode1[] = { 0xC1 };
InstructionInfo instInfo1 = { 0 };

UINT bytesProcessed1 = ParseModRM(testCode1, &instInfo1);

// 预期结果：
// bytesProcessed1 = 1
// instInfo1.modRM = 0xC1
// instInfo1.modRMFields.mod = 3
// instInfo1.modRMFields.reg = 0
// instInfo1.modRMFields.rm = 1
// instInfo1.displacementSize = 0
// instInfo1.isRelative = FALSE
```

## 测试用例2：无位移的内存寻址（mod=00）

```cpp
// 测试ModR/M字节：00 010 011 (0x13) - mod=00, reg=010, rm=011
// 表示无位移的内存寻址，如：add [ebx], edx
BYTE testCode2[] = { 0x13 };
InstructionInfo instInfo2 = { 0 };

UINT bytesProcessed2 = ParseModRM(testCode2, &instInfo2);

// 预期结果：
// bytesProcessed2 = 1
// instInfo2.modRM = 0x13
// instInfo2.modRMFields.mod = 0
// instInfo2.modRMFields.reg = 2
// instInfo2.modRMFields.rm = 3
// instInfo2.displacementSize = 0
// instInfo2.isRelative = FALSE
```

## 测试用例3：8位位移的内存寻址（mod=01）

```cpp
// 测试ModR/M字节：01 011 010 (0x5A) - mod=01, reg=011, rm=010
// 后跟8位位移0x12，表示带8位位移的内存寻址，如：add [edx+0x12], ebx
BYTE testCode3[] = { 0x5A, 0x12 };
InstructionInfo instInfo3 = { 0 };

UINT bytesProcessed3 = ParseModRM(testCode3, &instInfo3);

// 预期结果：
// bytesProcessed3 = 2
// instInfo3.modRM = 0x5A
// instInfo3.modRMFields.mod = 1
// instInfo3.modRMFields.reg = 3
// instInfo3.modRMFields.rm = 2
// instInfo3.displacement = 0x12
// instInfo3.displacementSize = 1
// instInfo3.operandOffset = 1
// instInfo3.isRelative = TRUE
```

## 测试用例4：32位位移的内存寻址（mod=10）

```cpp
// 测试ModR/M字节：10 100 110 (0x96) - mod=10, reg=100, rm=110
// 后跟32位位移0x12345678，表示带32位位移的内存寻址，如：add [esi+0x12345678], esp
BYTE testCode4[] = { 0x96, 0x78, 0x56, 0x34, 0x12 };
InstructionInfo instInfo4 = { 0 };

UINT bytesProcessed4 = ParseModRM(testCode4, &instInfo4);

// 预期结果：
// bytesProcessed4 = 5
// instInfo4.modRM = 0x96
// instInfo4.modRMFields.mod = 2
// instInfo4.modRMFields.reg = 4
// instInfo4.modRMFields.rm = 6
// instInfo4.displacement = 0x12345678
// instInfo4.displacementSize = 4
// instInfo4.operandOffset = 1
// instInfo4.isRelative = TRUE
```

## 测试用例5：直接寻址（mod=00, rm=101）

```cpp
// 测试ModR/M字节：00 111 101 (0x3D) - mod=00, reg=111, rm=101
// 后跟32位位移0x87654321，表示直接寻址，如：add [0x87654321], edi
BYTE testCode5[] = { 0x3D, 0x21, 0x43, 0x65, 0x87 };
InstructionInfo instInfo5 = { 0 };

UINT bytesProcessed5 = ParseModRM(testCode5, &instInfo5);

// 预期结果：
// bytesProcessed5 = 5
// instInfo5.modRM = 0x3D
// instInfo5.modRMFields.mod = 0
// instInfo5.modRMFields.reg = 7
// instInfo5.modRMFields.rm = 5
// instInfo5.displacement = 0x87654321
// instInfo5.displacementSize = 4
// instInfo5.operandOffset = 1
// instInfo5.isRelative = TRUE
```

## 测试用例6：SIB字节（mod=00, rm=100）

```cpp
// 测试ModR/M字节：00 001 100 (0x0C) - mod=00, reg=001, rm=100
// 后跟SIB字节：00 010 011 (0x13) - scale=0, index=2, base=3
// 表示使用SIB的内存寻址，如：add [ebx+ecx*1], ecx
BYTE testCode6[] = { 0x0C, 0x13 };
InstructionInfo instInfo6 = { 0 };

UINT bytesProcessed6 = ParseModRM(testCode6, &instInfo6);

// 预期结果：
// bytesProcessed6 = 2
// instInfo6.modRM = 0x0C
// instInfo6.modRMFields.mod = 0
// instInfo6.modRMFields.reg = 1
// instInfo6.modRMFields.rm = 4
// instInfo6.sib = 0x13
// instInfo6.sibFields.scale = 0
// instInfo6.sibFields.index = 2
// instInfo6.sibFields.base = 3
// instInfo6.displacementSize = 0
// instInfo6.isRelative = FALSE
```

## 测试用例7：SIB字节带32位位移（mod=00, rm=100, base=101）

```cpp
// 测试ModR/M字节：00 110 100 (0x34) - mod=00, reg=110, rm=100
// 后跟SIB字节：01 011 101 (0x5D) - scale=1, index=3, base=5
// 后跟32位位移0xAABBCCDD
// 表示使用SIB的特殊内存寻址，如：add [ebx*2+0xAABBCCDD], esi
BYTE testCode7[] = { 0x34, 0x5D, 0xDD, 0xCC, 0xBB, 0xAA };
InstructionInfo instInfo7 = { 0 };

UINT bytesProcessed7 = ParseModRM(testCode7, &instInfo7);

// 预期结果：
// bytesProcessed7 = 6
// instInfo7.modRM = 0x34
// instInfo7.modRMFields.mod = 0
// instInfo7.modRMFields.reg = 6
// instInfo7.modRMFields.rm = 4
// instInfo7.sib = 0x5D
// instInfo7.sibFields.scale = 1
// instInfo7.sibFields.index = 3
// instInfo7.sibFields.base = 5
// instInfo7.displacement = 0xAABBCCDD
// instInfo7.displacementSize = 4
// instInfo7.operandOffset = 2
// instInfo7.isRelative = TRUE
```

## 测试用例8：无效内存访问处理

```cpp
// 测试ModR/M字节：10 001 010 (0x8A) - mod=10, reg=001, rm=010
// 需要后跟32位位移，但内存不可读
// 模拟方法：使用MemoryUtils::IsMemoryReadable的模拟版本，在特定地址返回false
BYTE testCode8[] = { 0x8A }; // 故意不提供足够的字节
InstructionInfo instInfo8 = { 0 };

// 假设我们有一个模拟的IsMemoryReadable函数，在codePtr+1时返回false
UINT bytesProcessed8 = ParseModRM(testCode8, &instInfo8);

// 预期结果：
// bytesProcessed8 = 1 (只处理了ModR/M字节，因为后续内存不可读)
// instInfo8.modRM = 0x8A
// instInfo8.modRMFields.mod = 2
// instInfo8.modRMFields.reg = 1
// instInfo8.modRMFields.rm = 2
// instInfo8.displacementSize = 0 (未能读取位移)
// instInfo8.isRelative = FALSE
```

## 测试执行方法

可以创建一个单元测试函数，按顺序执行上述测试用例，并验证实际结果是否与预期结果一致。例如：

```cpp
void TestParseModRM()
{
    // 测试用例1：寄存器寻址
    BYTE testCode1[] = { 0xC1 };
    InstructionInfo instInfo1 = { 0 };
    UINT bytesProcessed1 = ParseModRM(testCode1, &instInfo1);
    assert(bytesProcessed1 == 1);
    assert(instInfo1.modRM == 0xC1);
    assert(instInfo1.modRMFields.mod == 3);
    assert(instInfo1.modRMFields.reg == 0);
    assert(instInfo1.modRMFields.rm == 1);
    assert(instInfo1.displacementSize == 0);
    assert(instInfo1.isRelative == FALSE);
    
    // 测试用例2：无位移的内存寻址
    // ...
    
    // 其他测试用例
    // ...
    
    printf("All ParseModRM tests passed!\n");
}
```

这些测试用例涵盖了ModR/M字节的各种组合和寻址模式，可以有效验证ParseModRM函数的正确性和健壮性。