#pragma once
#include <Windows.h>

class Logger; //前向声明

class MemoryUtils
{
public:
    static void SafeCloseHandle(HANDLE hHandle, HANDLE hResetValue = nullptr);

    // 检测地址是否可读
    static bool IsMemoryReadable(BYTE* ptr, SIZE_T size);

    // 检测地址是否可写
    static bool IsMemoryWritable(BYTE* ptr, SIZE_T size);

private:
    /*
     * 以下代码为规范性代码，静态工具类应避免显式生成对象，使用::的形式进行调用
     * 删除拷贝构造函数及拷贝赋值函数避免友元函数或成员函数进行拷贝操作
     */
    MemoryUtils() {} // 私有构造函数，防止外部调用构造
    ~MemoryUtils() {} // 私有析构函数，防止外部调用析构
    MemoryUtils(const MemoryUtils&) = delete; // 删除拷贝构造函数
    MemoryUtils& operator=(const MemoryUtils&) = delete; // 删除拷贝赋值操作
};

