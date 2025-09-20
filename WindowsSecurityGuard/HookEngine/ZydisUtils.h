#pragma once

#include "InstructionTypes.h"

// 前向声明：不透明结构体，隐藏内部实现
typedef struct ZydisContext* ZydisContextPtr;

/*
 * zydis库封装类：封装zydis各种操作实现指令解析、重定向功能
 */
class ZydisUtils
{
public:
    static ZydisContextPtr CreateContext(bool is64Bit);

    static void DestroyContext(ZydisContextPtr context);

    /*
     * @brief 使用Zydis库解析x86/x64指令，提取指令长度、类型和重定位信息
     * @param [IN] instructionBytes 指向待解析指令字节序列的指针，不能为空
     *        [IN] length 解析长度
     *        [OUT] parseLength 实际指令长度
     *        [OUT] zyContext 存储解析结果的结构体指针
     * @return 解析成功返回true，失败返回false（如指令无效、参数错误等）
     */
    static bool ParseInstruction(
        const void* instructionBytes,
        size_t length,
        size_t* parseLength,  // 实际解析长度
        ZydisContextPtr zyContext // !!!请注意，该参数在外部是不可读的
    );

    /**
     * @brief 重定位指令，根据指令类型将源地址的指令重定位到目标地址
     *
     * @param [IN] instruction   指令信息结构体
     *        [IN] sourceAddress 指令的原始地址
     *        [IN] targetAddress 指令将被重定位到的目标地址
     *        [IN] bufferSize    输出缓冲区的大小
     *        [OUT] outputSize   实际写入输出缓冲区的字节数
     * @return 重定位成功返回true，失败返回false
     */
    static bool RelocateInstruction(
        ZydisContextPtr zyData,
        BYTE* sourceAddress,
        BYTE* targetAddress,
        size_t* bufferSize,
        size_t* outputSize
    );

private:
    // 获取指令类型
    static InstructionType GetInstructionType(const ZydisContextPtr zyData);

    // 获取是否是相对寻址
    static bool IsRelativeInstruction(const ZydisContextPtr zyData);

    // 重定向近跳转指令
    static bool RelocateRelativeJump(
        ZydisContextPtr zyData, // 指令解析结果
        BYTE* sourceAddress,    // 原始指令地址
        BYTE* targetAddress,    // 修改后指令保存地址
        size_t* bufferSize,      // 输出缓冲区大小
        size_t* outputSize      // 实际写入大小
    );

    // 重定向远跳转（绝对地址）指令，通常转换为间接跳转
    static bool RelocateAbsoluteJump();

    // 重定向间接跳转指令
    static bool RelocateIndirectJump();

    // 重定向条件跳转指令
    static bool RelocateConditionalJump();

    // 重定向近调用指令
    static bool RelocateRelativeCall();

    // 重定向远调用（绝对地址）指令，通常转换为间接调用
    static bool RelocateAbsoluteCall();

    // 无需重定向，直接拷贝
    static bool CopyInstruction();

    // 检测RelativeJump参数是否合法
    static bool CheckRelativeJumpParam(
        BYTE* targetAddress,    // 修改后指令保存地址
        size_t* bufferSize,      // 输出缓冲区大小
        size_t needSize         // 需要的大小
    );

private:
    /*
     * 以下代码为规范性代码，工具类应避免显式生成对象，使用::的形式进行调用
     * 删除拷贝构造函数及拷贝赋值函数避免友元函数或成员函数进行拷贝操作
     */
    ZydisUtils() {} // 私有构造函数，防止外部调用构造
    ~ZydisUtils() {} // 私有析构函数，防止外部调用析构
    ZydisUtils(const ZydisUtils&) = delete; // 删除拷贝构造函数
    ZydisUtils& operator=(const ZydisUtils&) = delete; // 删除拷贝赋值操作
};

