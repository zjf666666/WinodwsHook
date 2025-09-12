#pragma once

#include <Windows.h>

/* 
 * ��Գ��������ȡ���ͷţ�ʹ��RAIIģʽ���з�װ
 * ȷ��������Ա���ȷ�ͷţ�����ά���ɱ���������Ϊ��©���µ��ڴ�й©����
 */

// Ԥ�������رպ���
template <typename HandleType>
BOOL DefaultHandleCloseFunc(HandleType handle)
{
    return CloseHandle(reinterpret_cast<HANDLE>(handle));
}

// ģ���಻֧��h+cpp�ļ�����ʽ
template <typename HandleType = HANDLE>
class HandleWrapper
{
public:
    HandleWrapper(HandleType handle = nullptr, BOOL(*closeFunc)(HandleType) = DefaultHandleCloseFunc<HandleType>) : 
        m_handle(handle), m_closeFunc(closeFunc)
    {}

    ~HandleWrapper()
    {
        Close();
    }

    // �����ƶ�
    HandleWrapper(HandleWrapper&& other) noexcept : 
        m_handle(other.m_handle), m_closeFunc(other.m_closeFunc)
    {
        other.m_handle = nullptr;
    }

    HandleWrapper& operator=(HandleWrapper&& other) noexcept
    {
        if (this != other)
        {
            m_handle = other.m_handle;
            m_closeFunc = other.m_closeFunc;
            other.m_handle = nullptr;
        }
        return *this;
    }

    // ��ȡԭʼ���
    // =����ֻ����һ�����ֵ�Ŀ�����ʵ����Դ��windows���ƣ���ʽΪ���ü���
    // ֻ�õ����ض���������DuplicateHandle�Ż��������ü���
    // closehandle�Ὣ���ü���-1������ֵ�����ľ������������һ��ֵ��close���ͷ�
    // ֻ��ͨ��DuplicateHandle���ӵľ���Ų��ᱻ�ͷ�
    HandleType Get() const
    {
        return m_handle;
    }

    // ����Ƿ�Ϸ�
    bool IsValid() const
    {
        return m_handle != nullptr && m_handle != INVALID_HANDLE_VALUE;
    }

    void Close()
    {
        if (IsValid() && m_closeFunc)
        {
            m_closeFunc(m_handle);
            m_handle = nullptr;
        }
    }

    // ���ú��������ڶ�����
    void Reset(HandleType newHandle)
    {
        if (m_handle == newHandle)
        {
            return;
        }
        Close();
        m_handle = newHandle;
    }

    // !!!ע�⣬�ú������ƻ�RAIIģʽ��������
    // ʹ�øú����󣬸��಻�ٸ��������ͷţ���Ҫ�������Լ�ά�������������
    HandleType Release()
    {
        HandleType tmp = m_handle;
        m_handle = nullptr;
        return tmp;
    }

private:
    // ��ֹ�������������ͷ�
    HandleWrapper(HandleWrapper&) = delete;
    HandleWrapper& operator=(HandleWrapper&) = delete;

private:
    HANDLE m_handle;
    BOOL(*m_closeFunc)(HandleType);
};
