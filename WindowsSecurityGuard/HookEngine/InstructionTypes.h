#pragma once
#include <Windows.h>

// ָ��ܹ�ö��
enum InstructionArchitecture {
    ARCH_X86,    // 32λ�ܹ�
    ARCH_X64     // 64λ�ܹ�
};

// ָ������ö��
enum InstructionType {
    INST_UNKNOWN,           // δָ֪��
    INST_NORMAL,            // ��ָͨ��������⴦��
    INST_JMP_NEAR,          // ����ת�����32λƫ�ƣ�
    INST_JMP_SHORT,         // ����ת�����8λƫ�ƣ�
    INST_CALL_NEAR,         // �����ã����32λƫ�ƣ�
    INST_RET,               // ����ָ��
    INST_COND_JMP,          // ������תָ��
    INST_RIP_RELATIVE,      // RIP���Ѱַָ���64λ��

    INST_JMP_FAR,           // Զ��ת
    INST_JMP_INDIRECT,
    INST_JCC,
    INST_CALL_FAR,
    INST_SYSCALL,
    INST_INT,
    INST_MOV
};

// ǰ׺����ֶ�
struct PrefixFields {
    BOOL isExists;         // �Ƿ����
    UINT size;             // ǰ׺����
    BOOL hasOperandSize;   // �Ƿ��в�������Сǰ׺(0x66)
    BOOL hasAddressSize;   // �Ƿ��е�ַ��Сǰ׺(0x67)
    BYTE segment;          // ��ǰ׺(0x26/0x2E/0x36/0x3E/0x64/0x65)
    BOOL hasLock;          // �Ƿ���LOCKǰ׺(0xF0)
    BYTE repeat;           // �ظ�ǰ׺(0xF2/0xF3)
    PrefixFields() : isExists(FALSE), size(0) {}
};

// ����������ֶ�
struct OpcodeFields {
    BOOL isExists;         // �Ƿ����
    UINT size;             // �����볤��
    BYTE opcode;           // ��������
    BYTE isExtended;       // �Ƿ�����չ������
    UINT extensionType;    // ����������
    OpcodeFields() : isExists(FALSE), size(0) {}
};

// ModR/M����ֶ�
struct ModRMFields {
    BOOL isExists;          // �Ƿ����
    UINT size;              // ModR/M����
    BYTE modRM;             // ModR/M�ֶ�����
    BYTE mod;               // mod�ֶ� Ѱַģʽ ��λ7-6��
    BYTE reg;               // reg/opcode�ֶ� �Ĵ�������չ������ ��λ5-3��
    BYTE rm;                // r/m�ֶ� ���mod�ֶ�ָ���Ĵ������ڴ�Ѱַ��ʽ ��λ2-0��
    BOOL isRelative;        // mod���ڴ�Ѱַ�Ƿ�Ϊ���Ѱַ������������ָ������Ѱַ����jmpָ�
    ModRMFields() : isExists(FALSE), size(0) {}
};

// SIB����ֶ�
struct SIBFields {
    BOOL isExists;          // �Ƿ����
    UINT size;              // SIB����
    BYTE sib;               // SIB�ֶ�����
    BYTE scale;             // scale�ֶ� ָ�������Ĵ������������ӣ�λ7-6��
    BYTE index;             // index�ֶ� ָ�������Ĵ��� ��λ5-3��
    BYTE base;              // base�ֶ� ָ����ַ�Ĵ��� ��λ2-0��
    SIBFields() : isExists(FALSE), size(0) {}
};

// Displacement����ֶ�
struct DisplacementFields {
    BOOL isExists;          // �Ƿ����
    UINT size;              // Displacement����
    INT32 displacement;     // Displacement����
    DisplacementFields() : isExists(FALSE), size(0) {}
};

// Immediate����ֶ�
struct ImmediateFields {
    BOOL isExists;          // �Ƿ����
    UINT size;              // Immediate��С
    INT64 immediate;        // Immediate����
    ImmediateFields() : isExists(FALSE), size(0) {}
};

// Rex����ֶ�
struct RexFields {
    BOOL hasREXPrefix;      // �Ƿ���REXǰ׺(0x40-0x4F)
    BOOL rexW;              // REX.Wλ����������С(0=Ĭ�ϣ�1=64λ)
    BOOL rexR;              // REX.Rλ��ModR/M.reg�ֶε���չ
    BOOL rexX;              // REX.Xλ��SIB.index�ֶε���չ
    BOOL rexB;              // REX.Bλ��ModR/M.rm��SIB.base�ֶε���չ
};

// VEX/EVEXǰ׺����ֶΣ�����AVX/AVX-512ָ���64λ��Ч��
struct VexFields {
    BOOL hasVEXPrefix;      // �Ƿ���VEXǰ׺
    BYTE vexLength;         // VEXǰ׺���ȣ�2��3�ֽڣ�
    BOOL vexR;              // VEX.Rλ����REX.R��ͬ
    BOOL vexX;              // VEX.Xλ����REX.X��ͬ
    BOOL vexB;              // VEX.Bλ����REX.B��ͬ
    BOOL vexW;              // VEX.Wλ����REX.W��ͬ����3�ֽ�VEX��Ч��
    BYTE vexL;              // VEX.Lλ���������ȣ�0=128λ��1=256λ��
    BYTE vexPP;             // VEX.pp�ֶΣ���Ч�ڴ�ͳǰ׺��00=�ޣ�01=66h��10=F3h��11=F2h��
    BYTE vexmmmmm;          // VEX.mmmmm�ֶΣ���������չ��00001=0Fh��00010=0F38h��00011=0F3Ah��
    BYTE vexvvvv;           // VEX.vvvv�ֶΣ������Դ�Ĵ���ָ����
};

// ָ����Ϣ�����ṹ��
struct InstructionInfo_study {
    BYTE* address;          // ָ���ַ
    UINT length;            // ָ��ȣ��ֽ�����
    BOOL isRelative;        // �Ƿ�Ϊ���Ѱַָ��
    InstructionType type;   // ָ������
    BYTE bytes[16];         // ָ��ԭʼ�ֽڣ����16�ֽڣ�
    InstructionArchitecture arch; // ָ��ܹ�

    // 64λ�����ֶ�
    BOOL isRIPRelative;     // �Ƿ�ΪRIP���Ѱַ����64λ��Ч��
    INT64 ripOffset;        // RIP���ƫ����������isRIPRelativeΪTRUEʱ��Ч��

    // ǰ׺ָ��
    PrefixFields prefix;

    // ����������ֶ�
    OpcodeFields opcode;

    // ModR/M����ֶ�
    ModRMFields modRM;

    // SIB����ֶ�
    SIBFields sib;

    // Displacement����ֶ�
    DisplacementFields displacement;

    // Immediate����ֶ�
    ImmediateFields immediate;
    // REXǰ׺����ֶΣ���64λ��Ч��
    RexFields rex;

    // VEX/EVEXǰ׺����ֶΣ�����AVX/AVX-512ָ���64λ��Ч��
    VexFields vex;
};

struct Zydis_InstructionInfo
{
    UINT8 length; // ָ���ܳ��ȣ��ֽ�����������ȷ����Ҫ���ݺ�������ָ�Χ
    bool isRelative; // �Ƿ�Ϊ�����ת�����ָ�true��ʾ��Ҫ�ض�λ����
    INT32 relativeOffset; // ���ƫ����������isRelativeΪtrueʱ��Ч�������ض�λ����
    UINT8 originalBytes[15]; // ԭʼָ���ֽڱ��ݣ����15�ֽڣ����ڹ������庯��
    InstructionType type;   // ָ������
};