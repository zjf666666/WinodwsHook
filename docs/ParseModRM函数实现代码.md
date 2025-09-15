# ParseModRM函数实现代码

以下是X86InstructionParser::ParseModRM函数的完整实现代码：

```cpp
UINT X86InstructionParser::ParseModRM(BYTE* codePtr, InstructionInfo* instInfo)
{
    // 检查参数有效性
    if (nullptr == codePtr || nullptr == instInfo)
    {
        return 0;
    }

    // 检查内存是否可读
    if (!MemoryUtils::IsMemoryReadable(codePtr, 1))
    {
        return 0;
    }

    // 初始化返回值（已处理的字节数）
    UINT bytesProcessed = 1;  // ModR/M字节本身
    
    // 读取ModR/M字节
    BYTE modRM = *codePtr;
    
    // 存储ModR/M完整字节
    instInfo->modRM = modRM;
    
    // 提取各个字段
    BYTE mod = (modRM >> 6) & 0x03;  // 高2位
    BYTE reg = (modRM >> 3) & 0x07;  // 中间3位
    BYTE rm = modRM & 0x07;          // 低3位
    
    // 存储ModR/M字段到指令信息结构体
    instInfo->modRMFields.mod = mod;
    instInfo->modRMFields.reg = reg;
    instInfo->modRMFields.rm = rm;
    
    // 处理SIB字节
    if (rm == 0x04 && mod != 0x03) {  // r/m = 100 (ESP/RSP) 且不是寄存器寻址模式
        // 检查内存是否可读
        if (!MemoryUtils::IsMemoryReadable(codePtr + bytesProcessed, 1))
        {
            return bytesProcessed;
        }
        
        // 读取SIB字节
        BYTE sib = *(codePtr + bytesProcessed);
        bytesProcessed++;
        
        // 存储SIB完整字节
        instInfo->sib = sib;
        
        // 提取SIB字段
        BYTE scale = (sib >> 6) & 0x03;  // 高2位
        BYTE index = (sib >> 3) & 0x07;  // 中间3位
        BYTE base = sib & 0x07;          // 低3位
        
        // 存储SIB字段到指令信息结构体
        instInfo->sibFields.scale = scale;
        instInfo->sibFields.index = index;
        instInfo->sibFields.base = base;
        
        // 特殊情况：当base=5且mod=0时，需要读取32位位移
        if (base == 0x05 && mod == 0x00)
        {
            // 检查内存是否可读
            if (!MemoryUtils::IsMemoryReadable(codePtr + bytesProcessed, 4))
            {
                return bytesProcessed;
            }
            
            // 读取32位位移
            instInfo->displacement = *(INT32*)(codePtr + bytesProcessed);
            instInfo->displacementSize = 4;
            bytesProcessed += 4;
        }
    }
    
    // 处理位移量
    if (mod == 0x01) {  // 8位位移
        // 检查内存是否可读
        if (!MemoryUtils::IsMemoryReadable(codePtr + bytesProcessed, 1))
        {
            return bytesProcessed;
        }
        
        // 读取8位位移并符号扩展为32位
        instInfo->displacement = *(INT8*)(codePtr + bytesProcessed);
        instInfo->displacementSize = 1;
        bytesProcessed += 1;
    } else if (mod == 0x02) {  // 32位位移
        // 检查内存是否可读
        if (!MemoryUtils::IsMemoryReadable(codePtr + bytesProcessed, 4))
        {
            return bytesProcessed;
        }
        
        // 读取32位位移
        instInfo->displacement = *(INT32*)(codePtr + bytesProcessed);
        instInfo->displacementSize = 4;
        bytesProcessed += 4;
    } else if (mod == 0x00 && rm == 0x05) {  // 直接寻址（32位位移）
        // 检查内存是否可读
        if (!MemoryUtils::IsMemoryReadable(codePtr + bytesProcessed, 4))
        {
            return bytesProcessed;
        }
        
        // 读取32位位移
        instInfo->displacement = *(INT32*)(codePtr + bytesProcessed);
        instInfo->displacementSize = 4;
        bytesProcessed += 4;
    }
    
    // 设置操作数偏移量（如果需要）
    if (instInfo->displacementSize > 0) {
        instInfo->operandOffset = bytesProcessed - instInfo->displacementSize;
    }
    
    // 检查是否为相对寻址指令
    if (mod != 0x03 && instInfo->displacementSize > 0) {
        instInfo->isRelative = TRUE;
    }
    
    return bytesProcessed;
}
```

## 函数说明

1. **参数检查**：首先检查传入的参数是否有效，确保代码指针和指令信息结构体不为空。

2. **内存可读性检查**：使用MemoryUtils::IsMemoryReadable函数检查内存是否可读，防止访问无效内存。

3. **ModR/M字节解析**：
   - 读取ModR/M字节
   - 提取mod、reg和rm字段
   - 将这些字段存储到指令信息结构体中

4. **SIB字节处理**：
   - 当r/m=4（ESP/RSP）且不是寄存器寻址模式时，需要解析SIB字节
   - 提取scale、index和base字段
   - 处理特殊情况：当base=5且mod=0时，需要读取32位位移

5. **位移量处理**：
   - mod=01：读取8位位移并符号扩展为32位
   - mod=02：读取32位位移
   - mod=00且r/m=05：读取32位位移（直接寻址）

6. **操作数偏移量设置**：如果指令包含位移量，设置操作数偏移量。

7. **相对寻址标志设置**：如果指令使用内存寻址且包含位移量，设置相对寻址标志。

8. **返回处理的字节数**：返回ModR/M字节、SIB字节和位移量的总字节数。

## 注意事项

1. **内存安全**：函数在访问内存前进行了可读性检查，防止访问无效内存导致崩溃。

2. **错误处理**：当遇到无效内存时，函数会提前返回已处理的字节数，而不是继续执行。

3. **符号扩展**：对于8位位移，使用INT8类型进行读取，确保正确的符号扩展。

4. **特殊情况处理**：函数处理了多种特殊情况，如SIB字节中的特殊base值和直接寻址模式。

5. **相对寻址检测**：函数设置了相对寻址标志，这对于指令重定位非常重要。