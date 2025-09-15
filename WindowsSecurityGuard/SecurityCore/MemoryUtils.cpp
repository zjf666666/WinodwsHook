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
         * volatile�ؼ������ã�
         * 1. ��ֹ�������Ż�
         * 2. ÿ�δ��ڴ����¶�ȡ������ʹ�üĴ����еĻ���ֵ 
         * 3. ��volatile�����Ĳ������������򵽱��volatile����֮ǰ��֮��
         */
        /*
         * ʹ�øùؼ���ԭ��
         * 1. �����tmpû�б�ʹ�ã��ڱ������Ż�����δ�����ܲ��ᱻִ��
         * 2. ������Ҫȷ�����Ƿ��ʵ���ʵ���ڴ棬�����ǻ��棬�����⽫ʧȥ����
         * 3. ��Windowsϵͳ����У�ĳЩ�ڴ��������ڴ�ӳ��I/O��Ӳ���Ĵ����ȣ��Ķ�ȡ��Ϊ��������ͨ�ڴ治ͬ��
         *    volatile ȷ������Щ��������Ķ�ȡ��Ԥ�ڽ��У����ᱻ�������Ż��ı䡣
         */
        volatile BYTE tmp; 
        for (int i = 0; i < size; ++i)
        {
            tmp = ptr[0];
        }
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) // EXCEPTION_EXECUTE_HANDLERֻҪ�����쳣�ͽ����쳣�����߼�
    {
        return false;
    }
    // C++��SEH���ṹ���쳣����ȷ���˴����쳣֮��һ��������쳣�����߼�����˲���Ҫָ��Ĭ�ϵķ���ֵ
}
