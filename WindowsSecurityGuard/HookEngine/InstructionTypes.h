#pragma once
#include <Windows.h>

// 指令架构枚举
enum InstructionArchitecture {
    ARCH_X86,    // 32位架构
    ARCH_X64     // 64位架构
};

// 指令类型枚举
enum InstructionType {
    INST_UNKNOWN,           // 未知指令
    INST_NORMAL,            // 普通指令（无需特殊处理）
    INST_JMP_NEAR,          // 近跳转（相对32位偏移）
    INST_JMP_SHORT,         // 短跳转（相对8位偏移）
    INST_CALL_NEAR,         // 近调用（相对32位偏移）
    INST_RET,               // 返回指令
    INST_COND_JMP,          // 条件跳转指令
    INST_RIP_RELATIVE,      // RIP相对寻址指令（仅64位）

    INST_JMP_FAR,           // 远跳转
    INST_JMP_INDIRECT,
    INST_JCC,
    INST_CALL_FAR,
    INST_SYSCALL,
    INST_INT,
    INST_MOV
};

// 前缀相关字段
struct PrefixFields {
    BOOL isExists;         // 是否存在
    UINT size;             // 前缀长度
    BOOL hasOperandSize;   // 是否有操作数大小前缀(0x66)
    BOOL hasAddressSize;   // 是否有地址大小前缀(0x67)
    BYTE segment;          // 段前缀(0x26/0x2E/0x36/0x3E/0x64/0x65)
    BOOL hasLock;          // 是否有LOCK前缀(0xF0)
    BYTE repeat;           // 重复前缀(0xF2/0xF3)
    PrefixFields() : isExists(FALSE), size(0) {}
};

// 操作码相关字段
struct OpcodeFields {
    BOOL isExists;         // 是否存在
    UINT size;             // 操作码长度
    BYTE opcode;           // 主操作码
    BYTE isExtended;       // 是否是扩展操作码
    UINT extensionType;    // 操作码类型
    OpcodeFields() : isExists(FALSE), size(0) {}
};

// ModR/M相关字段
struct ModRMFields {
    BOOL isExists;          // 是否存在
    UINT size;              // ModR/M长度
    BYTE modRM;             // ModR/M字段内容
    BYTE mod;               // mod字段 寻址模式 （位7-6）
    BYTE reg;               // reg/opcode字段 寄存器或扩展操作码 （位5-3）
    BYTE rm;                // r/m字段 结合mod字段指定寄存器或内存寻址方式 （位2-0）
    BOOL isRelative;        // mod的内存寻址是否为相对寻址，区别于整条指令的相对寻址（如jmp指令）
    ModRMFields() : isExists(FALSE), size(0) {}
};

// SIB相关字段
struct SIBFields {
    BOOL isExists;          // 是否存在
    UINT size;              // SIB长度
    BYTE sib;               // SIB字段内容
    BYTE scale;             // scale字段 指定索引寄存器的缩放因子（位7-6）
    BYTE index;             // index字段 指定索引寄存器 （位5-3）
    BYTE base;              // base字段 指定基址寄存器 （位2-0）
    SIBFields() : isExists(FALSE), size(0) {}
};

// Displacement相关字段
struct DisplacementFields {
    BOOL isExists;          // 是否存在
    UINT size;              // Displacement长度
    INT32 displacement;     // Displacement内容
    DisplacementFields() : isExists(FALSE), size(0) {}
};

// Immediate相关字段
struct ImmediateFields {
    BOOL isExists;          // 是否存在
    UINT size;              // Immediate大小
    INT64 immediate;        // Immediate内容
    ImmediateFields() : isExists(FALSE), size(0) {}
};

// Rex相关字段
struct RexFields {
    BOOL hasREXPrefix;      // 是否有REX前缀(0x40-0x4F)
    BOOL rexW;              // REX.W位：操作数大小(0=默认，1=64位)
    BOOL rexR;              // REX.R位：ModR/M.reg字段的扩展
    BOOL rexX;              // REX.X位：SIB.index字段的扩展
    BOOL rexB;              // REX.B位：ModR/M.rm或SIB.base字段的扩展
};

// VEX/EVEX前缀相关字段（用于AVX/AVX-512指令，仅64位有效）
struct VexFields {
    BOOL hasVEXPrefix;      // 是否有VEX前缀
    BYTE vexLength;         // VEX前缀长度（2或3字节）
    BOOL vexR;              // VEX.R位：与REX.R相同
    BOOL vexX;              // VEX.X位：与REX.X相同
    BOOL vexB;              // VEX.B位：与REX.B相同
    BOOL vexW;              // VEX.W位：与REX.W相同（仅3字节VEX有效）
    BYTE vexL;              // VEX.L位：向量长度（0=128位，1=256位）
    BYTE vexPP;             // VEX.pp字段：等效于传统前缀（00=无，01=66h，10=F3h，11=F2h）
    BYTE vexmmmmm;          // VEX.mmmmm字段：操作码扩展（00001=0Fh，00010=0F38h，00011=0F3Ah）
    BYTE vexvvvv;           // VEX.vvvv字段：额外的源寄存器指定符
};

// 指令信息基础结构体
struct InstructionInfo_study {
    BYTE* address;          // 指令地址
    UINT length;            // 指令长度（字节数）
    BOOL isRelative;        // 是否为相对寻址指令
    InstructionType type;   // 指令类型
    BYTE bytes[16];         // 指令原始字节（最大16字节）
    InstructionArchitecture arch; // 指令架构

    // 64位特有字段
    BOOL isRIPRelative;     // 是否为RIP相对寻址（仅64位有效）
    INT64 ripOffset;        // RIP相对偏移量（仅当isRIPRelative为TRUE时有效）

    // 前缀指令
    PrefixFields prefix;

    // 操作码相关字段
    OpcodeFields opcode;

    // ModR/M相关字段
    ModRMFields modRM;

    // SIB相关字段
    SIBFields sib;

    // Displacement相关字段
    DisplacementFields displacement;

    // Immediate相关字段
    ImmediateFields immediate;
    // REX前缀相关字段（仅64位有效）
    RexFields rex;

    // VEX/EVEX前缀相关字段（用于AVX/AVX-512指令，仅64位有效）
    VexFields vex;
};

struct Zydis_InstructionInfo
{
    UINT8 length; // 指令总长度（字节数），用于确定需要备份和跳过的指令范围
    bool isRelative; // 是否为相对跳转或调用指令，true表示需要重定位处理
    INT32 relativeOffset; // 相对偏移量，仅当isRelative为true时有效，用于重定位计算
    UINT8 originalBytes[15]; // 原始指令字节备份，最多15字节，用于构建跳板函数
    InstructionType type;   // 指令类型
};