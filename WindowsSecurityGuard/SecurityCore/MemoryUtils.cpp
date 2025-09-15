#include "pch.h"
#include "MemoryUtils.h"
#include "Logger.h"

void MemoryUtils::SafeCloseHandle(HANDLE hHandle, HANDLE hResetValue)
{
    if (nullptr != hHandle && INVALID_HANDLE_VALUE != hHandle)
    {
        CloseHandle(hHandle);
        hHandle = hResetValue;
    }
}

bool MemoryUtils::IsMemoryReadable(BYTE* ptr, SIZE_T size)
{
    __try
    {
        /*
         * volatile关键字作用：
         * 1. 禁止编译器优化
         * 2. 每次从内存重新读取，而不使用寄存器中的缓存值 
         * 3. 对volatile变量的操作不会重排序到别的volatile变量之前或之后
         */
        /*
         * 使用该关键字原因：
         * 1. 这里的tmp没有被使用，在编译器优化后，这段代码可能不会被执行
         * 2. 我们需要确保我们访问的是实际内存，而不是缓存，否则检测将失去意义
         * 3. 在Windows系统编程中，某些内存区域（如内存映射I/O、硬件寄存器等）的读取行为可能与普通内存不同。
         *    volatile 确保对这些特殊区域的读取按预期进行，不会被编译器优化改变。
         */
        volatile BYTE tmp; 
        for (int i = 0; i < size; ++i)
        {
            tmp = ptr[0];
        }
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) // EXCEPTION_EXECUTE_HANDLER只要出现异常就进入异常处理逻辑
    {
        return false;
    }
    // C++的SEH（结构化异常处理）确保了触发异常之后一定会进入异常处理逻辑，因此不需要指定默认的返回值
}
