#pragma once

#include "X86InstructionParser.h"
#include "X64InstructionParser.h"

class InstructionParserFactory {
public:
    // ���ݼܹ�������Ӧ�Ľ�����
    static IInstructionParser* CreateParser(InstructionArchitecture arch) {
        switch (arch) {
        case ARCH_X86:
            return new X86InstructionParser();
        case ARCH_X64:
            return new X64InstructionParser();
        default:
            return nullptr;
        }
    }
};