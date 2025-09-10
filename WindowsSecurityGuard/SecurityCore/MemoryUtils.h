#pragma once
#include <Windows.h>

class MemoryUtils
{
    /*
     * @brief 从指定进程的内存地址读取数据
     * @param [IN] hProcess 目标进程的句柄，需要具有PROCESS_VM_READ权限
     *        [IN] lpBaseAddress 要读取的内存起始地址
     *        [OUT] lpBuffer 接收读取数据的缓冲区
     *        [IN] nSize 要读取的字节数
     *        [OUT] lpNumberOfBytesRead 实际读取的字节数
     * @return BOOL 成功返回TRUE，失败返回FALSE
     */
    static BOOL ReadMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead);

    /*
     * @brief 向指定进程的内存地址写入数据
     * @param [IN] hProcess 目标进程的句柄，需要具有PROCESS_VM_WRITE和PROCESS_VM_OPERATION权限
     *        [IN] lpBaseAddress 要写入的内存起始地址
     *        [IN] lpBuffer 包含要写入数据的缓冲区
     *        [IN] nSize 要写入的字节数
     *        [OUT] lpNumberOfBytesWritten 实际写入的字节数
     * @return BOOL 成功返回TRUE，失败返回FALSE
     */
    static BOOL WriteMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);

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

