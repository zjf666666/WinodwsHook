# ModRM字段添加说明

为了正确实现ParseModRM函数，需要在InstructionInfo结构体中添加ModRM相关的字段。以下是建议添加的字段：

```cpp
// ModR/M字段相关
BYTE modRM;                  // ModR/M字节的完整值
struct {
    BYTE mod;               // ModR/M.mod字段 (位7-6)
    BYTE reg;               // ModR/M.reg字段 (位5-3)
    BYTE rm;                // ModR/M.rm字段 (位2-0)
} modRMFields;

// SIB字段相关
BYTE sib;                    // SIB字节的完整值
struct {
    BYTE scale;            // SIB.scale字段 (位7-6)
    BYTE index;            // SIB.index字段 (位5-3)
    BYTE base;             // SIB.base字段 (位2-0)
} sibFields;

// 位移量相关
INT32 displacement;          // 位移量值
BYTE displacementSize;       // 位移量大小（0, 1, 2或4字节）
```

这些字段应该添加到InstructionInfo结构体中，位于操作码相关字段之后，64位特有字段之前。

添加这些字段后，ParseModRM函数将能够正确解析ModR/M字节，并将解析结果存储在InstructionInfo结构体中。