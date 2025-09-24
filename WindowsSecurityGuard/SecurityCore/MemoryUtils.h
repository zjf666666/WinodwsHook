#pragma once
#include <Windows.h>

class Logger; //ǰ������

class MemoryUtils
{
public:
    static void SafeCloseHandle(HANDLE hHandle, HANDLE hResetValue = nullptr);

    // ����ַ�Ƿ�ɶ�
    static bool IsMemoryReadable(BYTE* ptr, SIZE_T size);

    // ����ַ�Ƿ��д
    static bool IsMemoryWritable(BYTE* ptr, SIZE_T size);

private:
    /*
     * ���´���Ϊ�淶�Դ��룬��̬������Ӧ������ʽ���ɶ���ʹ��::����ʽ���е���
     * ɾ���������캯����������ֵ����������Ԫ�������Ա�������п�������
     */
    MemoryUtils() {} // ˽�й��캯������ֹ�ⲿ���ù���
    ~MemoryUtils() {} // ˽��������������ֹ�ⲿ��������
    MemoryUtils(const MemoryUtils&) = delete; // ɾ���������캯��
    MemoryUtils& operator=(const MemoryUtils&) = delete; // ɾ��������ֵ����
};

