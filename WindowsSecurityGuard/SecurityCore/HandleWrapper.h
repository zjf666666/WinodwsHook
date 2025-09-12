#pragma once

#include <Windows.h>

/* 
 * 针对常见句柄获取与释放，使用RAII模式进行封装
 * 确保句柄可以被正确释放，减少维护成本，减少人为遗漏导致的内存泄漏问题
 */

// 预定义句柄关闭函数
template <typename HandleType>
BOOL DefaultHandleCloseFunc(HandleType handle)
{
    return CloseHandle(reinterpret_cast<HANDLE>(handle));
}

// 模板类不支持h+cpp文件的形式
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

    // 允许移动
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

    // 获取原始句柄
    // =操作只能做一个句柄值的拷贝，实际资源由windows控制，方式为引用计数
    // 只用调用特定函数，如DuplicateHandle才会增加引用计数
    // closehandle会将引用计数-1，所以值拷贝的句柄会随着任意一个值的close而释放
    // 只有通过DuplicateHandle增加的句柄才不会被释放
    HandleType Get() const
    {
        return m_handle;
    }

    // 句柄是否合法
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

    // 重置函数，用于对象复用
    void Reset(HandleType newHandle)
    {
        if (m_handle == newHandle)
        {
            return;
        }
        Close();
        m_handle = newHandle;
    }

    // !!!注意，该函数会破坏RAII模式，请慎用
    // 使用该函数后，该类不再负责句柄的释放，需要调用这自己维护句柄生命周期
    HandleType Release()
    {
        HandleType tmp = m_handle;
        m_handle = nullptr;
        return tmp;
    }

private:
    // 禁止拷贝，避免多次释放
    HandleWrapper(HandleWrapper&) = delete;
    HandleWrapper& operator=(HandleWrapper&) = delete;

private:
    HANDLE m_handle;
    BOOL(*m_closeFunc)(HandleType);
};
