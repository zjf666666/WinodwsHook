#pragma once
#include "pch.h"

#include <stdexcept>

#include <Windows.h>

#include "Logger.h"
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
    ) : m_hProcess(GetCurrentProcess()), m_lpAddr(lpAddress), m_size(0)
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
    ) : m_hProcess(hProcess), m_lpAddr(lpAddress), m_size(0)
    {
        if (nullptr == hProcess || INVALID_HANDLE_VALUE == hProcess)
        {
            throw std::invalid_argument("Invalid process handle");
        }
        allocate(lpAddress, dwSize, flAllocationType, flProtect);
    }

    ~VirtualAllocWrapper()
    {
        Free();
    }

    // ���ÿ�����������ֹ�ظ��ͷ�
    VirtualAllocWrapper(VirtualAllocWrapper&) = delete;
    VirtualAllocWrapper& operator=(VirtualAllocWrapper&) = delete;

    // �����ƶ�
    VirtualAllocWrapper(VirtualAllocWrapper&& other) noexcept :
        m_lpAddr(other.m_lpAddr), m_hProcess(other.m_hProcess), m_size(other.m_size)
    {
        other.m_lpAddr = nullptr;
        other.m_size = 0;
    }

    VirtualAllocWrapper& operator=(VirtualAllocWrapper&& other)
    {
        if (this != &other)
        {
            Free();
            m_lpAddr = other.m_lpAddr;
            m_hProcess = other.m_hProcess;
            m_size = other.m_size;
            other.m_lpAddr = nullptr;
            other.m_size = 0;
        }
        return *this;
    }

    LPVOID Get() const
    {
        return m_lpAddr;
    }

    SIZE_T size()
    {
        return m_size;
    }

    bool IsValid() const
    {
        return m_lpAddr != nullptr;
    }

private:
    // �����ڴ溯��
    void allocate(
        LPVOID lpAddress,
        SIZE_T dwSize,
        DWORD flAllocationType,
        DWORD flProtect
    )
    {
        if (0 == dwSize)
        {
            throw std::invalid_argument("Allocation size cannot be zero");
        }

        if (GetCurrentProcess() == m_hProcess)
        {
            m_lpAddr = VirtualAlloc(lpAddress, dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (nullptr == m_lpAddr)
            {
                Logger::GetInstance().Error(L"VirtualAlloc failed! error = %d", GetLastError());
                throw std::runtime_error("VirtualAlloc failed");
            }
            m_size = dwSize;
        }
        if (INVALID_HANDLE_VALUE == m_hProcess || nullptr == m_hProcess)
        {
            Logger::GetInstance().Error(L"m_hProcess is invalid");
            throw std::invalid_argument("VirtualAlloc failed");
        }
        m_lpAddr = VirtualAllocEx(m_hProcess, lpAddress, dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (nullptr == m_lpAddr);
        {
            Logger::GetInstance().Error(L"VirtualFreeEx failed! error = %d", GetLastError());
        }
    }

    // Free�������ϸ��ֹʹ���쳣����C++��׼�涨�����������׳�δ������쳣�ᵼ�³�����ֹ
    void Free()
    {
        do
        {
            if (!IsValid()) // ��Դ���Ϸ����ͷ��κ���Դ
            {
                break;
            }
            if (GetCurrentProcess() == m_hProcess)
            {
                if (FALSE == VirtualFree(m_lpAddr, m_size, MEM_RELEASE));
                {
                    Logger::GetInstance().Error(L"VirtualFree failed! error = %d", GetLastError());
                }
                break;
            }
            if (INVALID_HANDLE_VALUE == m_hProcess || nullptr == m_hProcess)
            {
                Logger::GetInstance().Error(L"m_hProcess is invalid");
                break;
            }
            if (FALSE == VirtualFreeEx(m_hProcess, m_lpAddr, m_size, MEM_RELEASE));
            {
                Logger::GetInstance().Error(L"VirtualFreeEx failed! error = %d", GetLastError());
                break;
            }
        } while (false);
        m_lpAddr = nullptr; // �����Ƿ�ɹ���������Դ�ÿգ������ܼ����ڴ�й¶����
        m_size = 0;
    }

private:
    LPVOID m_lpAddr;
    HANDLE m_hProcess;
    SIZE_T m_size;
};

// virtualprotect
class VirtualProtectWrapper
{
public:
    VirtualProtectWrapper(
        LPVOID lpAddress,
        SIZE_T dwSize,
        DWORD flNewProtect
    ) : m_lpAddr(nullptr), m_dwSize(0)
    {
        if (nullptr == lpAddress)
        {
            throw std::invalid_argument("Invalid address");
        }

        if (0 == dwSize)
        {
            throw std::invalid_argument("Change size cannot be zero");
        }

        if (FALSE == VirtualProtect(lpAddress, dwSize, flNewProtect, &m_dwOldFlag))
        {
            Logger::GetInstance().Error(L"VirtualProtect failed! error = %d", GetLastError());
            throw std::runtime_error("VirtualProtect failed");
        }

        m_lpAddr = lpAddress;
        m_dwSize = dwSize;
    }

    ~VirtualProtectWrapper()
    {
        Recover();
    }

    // ��ֹ���������������ظ�����
    VirtualProtectWrapper(VirtualProtectWrapper&) = delete;
    VirtualProtectWrapper& operator=(VirtualProtectWrapper&) = delete;

    // �����ƶ�
    VirtualProtectWrapper(VirtualProtectWrapper&& other) noexcept :
        m_dwOldFlag(other.m_dwOldFlag), m_lpAddr(other.m_lpAddr), m_dwSize(other.m_dwSize)
    {
        other.m_lpAddr = nullptr;
        other.m_dwSize = 0;
    }

    VirtualProtectWrapper& operator=(VirtualProtectWrapper&& other) noexcept
    {
        if (this != &other)
        {
            Recover();
            m_dwOldFlag = other.m_dwOldFlag;
            m_dwSize = other.m_dwSize;
            m_lpAddr = other.m_lpAddr;
            other.m_lpAddr = nullptr;
            other.m_dwSize = 0;
        }
        return *this;
    }

    bool IsValid() const
    {
        return m_lpAddr != nullptr;
    }

    LPVOID Get() const
    {
        return m_lpAddr;
    }

private:
    // Recover�������ϸ��ֹʹ���쳣����C++��׼�涨�����������׳�δ������쳣�ᵼ�³�����ֹ
    void Recover()
    {
        do
        {
            if (!IsValid())
            {
                Logger::GetInstance().Error(L"Addr is nullptr!");
                break;
            }

            if (0 == m_dwSize)
            {
                Logger::GetInstance().Error(L"Size is zero!");
                break;
            }

            if (FALSE == VirtualProtect(m_lpAddr, m_dwSize, m_dwOldFlag, &m_dwOldFlag))
            {
                Logger::GetInstance().Error(L"VirtualProtect failed! error = %d", GetLastError());
                break;
            }
        } while (false);
        m_lpAddr = nullptr;  // �����Ƿ�ɹ���������Դ�ÿգ������ܼ����ڴ�й¶����
        m_dwSize = 0;
    }

private:
    DWORD m_dwOldFlag;
    LPVOID m_lpAddr;
    SIZE_T m_dwSize;
};