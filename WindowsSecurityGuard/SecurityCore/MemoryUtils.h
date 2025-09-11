#pragma once
#include <Windows.h>

class Logger; //前向声明

// 这个类的操作有待考量，当前实现没有实际意义
class MemoryUtils
{
public:
    /*
     * @brief 从指定进程的内存地址读取数据，暂不实现
     * @param [IN] hProcess 目标进程的句柄，需要具有PROCESS_VM_READ权限
     *        [IN] lpBaseAddress 要读取的内存起始地址
     *        [OUT] lpBuffer 接收读取数据的缓冲区
     *        [IN] nSize 要读取的字节数
     *        [OUT] lpNumberOfBytesRead 实际读取的字节数
     * @return bool 成功返回true，失败返回false
     */
    static bool ReadMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead);

    /*
     * @brief 向指定进程的内存地址写入数据
     * @param [IN] hProcess 目标进程的句柄，需要具有PROCESS_VM_WRITE和PROCESS_VM_OPERATION权限
     *        [IN] lpBaseAddress 要写入的内存起始地址
     *        [IN] lpBuffer 包含要写入数据的缓冲区
     *        [IN] nSize 要写入的字节数
     *        [OUT] lpNumberOfBytesWritten 实际写入的字节数
     *        [IN] flAllocationType 内存分配属性
     *        [IN] flProtect 内存保护属性
     *        [IN] bIsRealloc 指定地址分配失败是否使用系统默认分配重新分配一次
     * @return bool 成功返回true，失败返回false
     */
    static bool WriteMemory(
        HANDLE hProcess,
        LPVOID lpBaseAddress,
        LPCVOID lpBuffer,
        SIZE_T nSize,
        SIZE_T* lpNumberOfBytesWritten,
        DWORD flAllocationType = MEM_COMMIT | MEM_RESERVE,
        DWORD flProtect = PAGE_READWRITE,
        bool bIsRealloc = false
        );

    static void SafeCloseHandle(HANDLE hHandle, HANDLE hResetValue = nullptr);

private:
    /*
     * 以下代码为规范性代码，工具类应避免显式生成对象，使用::的形式进行调用
     * 删除拷贝构造函数及拷贝赋值函数避免友元函数或成员函数进行拷贝操作
     */
    MemoryUtils() {} // 私有构造函数，防止外部调用构造
    ~MemoryUtils() {} // 私有析构函数，防止外部调用析构
    MemoryUtils(const MemoryUtils&) = delete; // 删除拷贝构造函数
    MemoryUtils& operator=(const MemoryUtils&) = delete; // 删除拷贝赋值操作
};

