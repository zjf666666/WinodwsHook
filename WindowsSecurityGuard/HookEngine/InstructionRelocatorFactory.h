#pragma once

#include "X86InstructionRelocator.h"
#include "X64InstructionRelocator.h"

#include "X86InstructionParser.h"
#include "X64InstructionParser.h"

// 重定位器工厂
class InstructionRelocatorFactory {
public:
    // 根据架构创建对应的重定位器
    static IInstructionRelocator* CreateRelocator(InstructionArchitecture arch, IInstructionParser* parser) {
        switch (arch) {
        case ARCH_X86:
            return new X86InstructionRelocator(static_cast<X86InstructionParser*>(parser));
        case ARCH_X64:
            return new X64InstructionRelocator(static_cast<X64InstructionParser*>(parser));
        default:
            return nullptr;
        }
    }
};