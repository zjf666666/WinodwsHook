#pragma once
#include <Windows.h>

class Logger; //ǰ������

// �����Ĳ����д���������ǰʵ��û��ʵ������
class MemoryUtils
{
public:
    /*
     * @brief ��ָ�����̵��ڴ��ַ��ȡ���ݣ��ݲ�ʵ��
     * @param [IN] hProcess Ŀ����̵ľ������Ҫ����PROCESS_VM_READȨ��
     *        [IN] lpBaseAddress Ҫ��ȡ���ڴ���ʼ��ַ
     *        [OUT] lpBuffer ���ն�ȡ���ݵĻ�����
     *        [IN] nSize Ҫ��ȡ���ֽ���
     *        [OUT] lpNumberOfBytesRead ʵ�ʶ�ȡ���ֽ���
     * @return bool �ɹ�����true��ʧ�ܷ���false
     */
    static bool ReadMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead);

    /*
     * @brief ��ָ�����̵��ڴ��ַд������
     * @param [IN] hProcess Ŀ����̵ľ������Ҫ����PROCESS_VM_WRITE��PROCESS_VM_OPERATIONȨ��
     *        [IN] lpBaseAddress Ҫд����ڴ���ʼ��ַ
     *        [IN] lpBuffer ����Ҫд�����ݵĻ�����
     *        [IN] nSize Ҫд����ֽ���
     *        [OUT] lpNumberOfBytesWritten ʵ��д����ֽ���
     *        [IN] flAllocationType �ڴ��������
     *        [IN] flProtect �ڴ汣������
     *        [IN] bIsRealloc ָ����ַ����ʧ���Ƿ�ʹ��ϵͳĬ�Ϸ������·���һ��
     * @return bool �ɹ�����true��ʧ�ܷ���false
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
     * ���´���Ϊ�淶�Դ��룬������Ӧ������ʽ���ɶ���ʹ��::����ʽ���е���
     * ɾ���������캯����������ֵ����������Ԫ�������Ա�������п�������
     */
    MemoryUtils() {} // ˽�й��캯������ֹ�ⲿ���ù���
    ~MemoryUtils() {} // ˽��������������ֹ�ⲿ��������
    MemoryUtils(const MemoryUtils&) = delete; // ɾ���������캯��
    MemoryUtils& operator=(const MemoryUtils&) = delete; // ɾ��������ֵ����
};

