#pragma once
#include <Windows.h>

class MemoryUtils
{
    /*
     * @brief ��ָ�����̵��ڴ��ַ��ȡ����
     * @param [IN] hProcess Ŀ����̵ľ������Ҫ����PROCESS_VM_READȨ��
     *        [IN] lpBaseAddress Ҫ��ȡ���ڴ���ʼ��ַ
     *        [OUT] lpBuffer ���ն�ȡ���ݵĻ�����
     *        [IN] nSize Ҫ��ȡ���ֽ���
     *        [OUT] lpNumberOfBytesRead ʵ�ʶ�ȡ���ֽ���
     * @return BOOL �ɹ�����TRUE��ʧ�ܷ���FALSE
     */
    static BOOL ReadMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead);

    /*
     * @brief ��ָ�����̵��ڴ��ַд������
     * @param [IN] hProcess Ŀ����̵ľ������Ҫ����PROCESS_VM_WRITE��PROCESS_VM_OPERATIONȨ��
     *        [IN] lpBaseAddress Ҫд����ڴ���ʼ��ַ
     *        [IN] lpBuffer ����Ҫд�����ݵĻ�����
     *        [IN] nSize Ҫд����ֽ���
     *        [OUT] lpNumberOfBytesWritten ʵ��д����ֽ���
     * @return BOOL �ɹ�����TRUE��ʧ�ܷ���FALSE
     */
    static BOOL WriteMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);

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

