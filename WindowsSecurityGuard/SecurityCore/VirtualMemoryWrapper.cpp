#pragma once

#include <stdexcept>

#include <Windows.h>

/*
 * 针对常见进程内存操作使用RAII模式进行封装
 * 包含virtualalloc、virtualprotect等虚拟内存相关操作
 */

// 申请内存操作
class VirtualAllocWrapper
{
public:
    // VirtualAlloc函数
    VirtualAllocWrapper(
        LPVOID lpAddress,
        SIZE_T dwSize,
        DWORD flAllocationType,
        DWORD flProtect
    ) : m_hProcess(GetCurrentProcess()), m_lpAddr(lpAddress)
    {
        allocate(lpAddress, dwSize, flAllocationType, flProtect);
    }

    // VirtualAllocEx函数
    VirtualAllocWrapper(
        HANDLE hProcess,
        LPVOID lpAddress,
        SIZE_T dwSize,
        DWORD flAllocationType,
        DWORD flProtect
    ) : m_hProcess(GetCurrentProcess()), m_lpAddr(lpAddress)
    {
        if (nullptr == hProcess || INVALID_HANDLE_VALUE == hProcess)
        {
            throw std::invalid_argument("Invalid process handle");
        }
        allocate(lpAddress, dwSize, flAllocationType, flProtect);
    }


private:
    // 禁用拷贝操作，防止重复释放
    VirtualAllocWrapper(VirtualAllocWrapper&) = delete;
    VirtualAllocWrapper& operator=(VirtualAllocWrapper&) = delete;

private:
    void allocate(
        LPVOID lpAddress,
        SIZE_T dwSize,
        DWORD flAllocationType,
        DWORD flProtect
    );

private:
    LPVOID m_lpAddr;
    HANDLE m_hProcess;
    SIZE_T m_size;
};