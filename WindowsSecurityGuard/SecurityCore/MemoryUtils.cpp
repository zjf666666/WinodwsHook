#include "MemoryUtils.h"
#include "Logger.h"

bool MemoryUtils::ReadMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead)
{
    
    return true;
}

bool MemoryUtils::WriteMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T nSize,
    SIZE_T* lpNumberOfBytesWritten,
    DWORD flAllocationType,
    DWORD flProtect,
    bool bIsRealloc
)
{
    // �����Ϸ���У��
    if (NULL == hProcess || NULL == lpBuffer || 0 == nSize)
    {
        Logger::GetInstance().Error(L"WriteMemory param error!");
        return false;
    }

    // �ڽ����ڲ�����д����ڴ�ռ�
    LPVOID lpAddr = VirtualAllocEx(hProcess, lpBaseAddress, nSize, flAllocationType, flProtect);
    if (NULL == lpAddr)
    {
        if (bIsRealloc)
        {
            lpAddr = VirtualAllocEx(hProcess, NULL, nSize, flAllocationType, flProtect);
        }
        if (NULL == lpAddr)
        {
            Logger::GetInstance().Error(L"VirtualAllocEx failed! error = %d", GetLastError());
            return false;
        }
    }

    if (FALSE == WriteProcessMemory(hProcess, lpAddr, lpBuffer, nSize, lpNumberOfBytesWritten))
    {
        Logger::GetInstance().Error(L"WriteProcessMemory failed! error = %d", GetLastError());
        VirtualFreeEx(hProcess, lpAddr, nSize, MEM_RELEASE);
        return false;
    }

    VirtualFreeEx(hProcess, lpAddr, nSize, MEM_RELEASE);
    return true;
}

void MemoryUtils::SafeCloseHandle(HANDLE hHandle, HANDLE hResetValue)
{
    if (NULL != hHandle && INVALID_HANDLE_VALUE != hHandle)
    {
        CloseHandle(hHandle);
        hHandle = hResetValue;
    }
}
