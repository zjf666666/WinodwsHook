#include "pch.h"
#include "gtest/gtest.h"
#include "ZydisUtils.h"

TEST(ZydisUtils, ParseInstruction) {
    ZydisContextPtr ptr = ZydisUtils::CreateContext(true);
    uint8_t data1[] = {
    0x8B, 0x45, 0x08,  // ʵ��ָ�mov eax, [ebp+0x08]��3�ֽڣ�
    0x90, 0x90, 0x90, 0x90, 0x90,  // �����޹����ݣ�NOP��
    0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90, 0x90, 0x90, 0x90,
    0x90, 0x90  // �ܳ���20�ֽ�
    };
    size_t parseLen = 0;
    bool bRes = ZydisUtils::ParseInstruction(data1, sizeof(data1), &parseLen, ptr);
    //ZydisUtils::RelocateInstruction(ptr,);
    EXPECT_EQ(bRes, true);
}

//TEST(ZydisUtils, RelocateRelativeJump) {
//    ZydisContextPtr ptr = ZydisUtils::CreateContext(true);
//    uint8_t data1[] = {
//        0xEB, 0x05
//    };
//    size_t parseLen = 0;
//    bool bRes = ZydisUtils::ParseInstruction(data1, sizeof(data1), &parseLen, ptr);
//    BYTE* sourceAddress = reinterpret_cast<BYTE*>(0x00401000);
//    BYTE* targetAddress = reinterpret_cast<BYTE*>(0x00502000);
//    size_t bufferSize = 5, outputSize = 0;
//    ZydisUtils::RelocateInstruction(ptr, sourceAddress, targetAddress, &bufferSize, &outputSize);
//    EXPECT_EQ(bRes, true);
//}