#pragma once

#include <stdexcept>

#include <Windows.h>

/*
 * ��Գ��������ڴ����ʹ��RAIIģʽ���з�װ
 * ����virtualalloc��virtualprotect�������ڴ���ز���
 */

// �����ڴ����
class VirtualAllocWrapper
{
public:
    // VirtualAlloc����
    VirtualAllocWrapper(
        LPVOID lpAddress,
        SIZE_T dwSize,
        DWORD flAllocationType,
        DWORD flProtect
    ) : m_hProcess(GetCurrentProcess()), m_lpAddr(lpAddress)
    {
        allocate(lpAddress, dwSize, flAllocationType, flProtect);
    }

    // VirtualAllocEx����
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
    // ���ÿ�����������ֹ�ظ��ͷ�
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