#include "pch.h"
#include "IATHook.h"

#include <string>
#include <algorithm>

#include <Windows.h>

#include "../SecurityCore/VirtualMemoryWrapper.h"
#include "../SecurityCore/Logger.h"
#include "../SecurityCore/StringUtils.h"

bool IATHook::Init(const Param& params)
{
    m_pOriginalFunction = nullptr;
    m_bIsInstalled = false;

    auto strArch = params.Get<std::string>("common_architecture");
    auto hookFunc = params.Get<void*>("iat_function_address");
    auto targetFunc = params.Get<std::string>("iat_function_name");
    auto targetModule = params.Get<std::wstring>("common_target_module");
    if (!strArch || !hookFunc || !targetFunc || !targetModule)
    {
        Logger::GetInstance().Error(L"Get params failed!");
        return false;
    }
    m_bIs64Bit = *strArch == "x64" ? true : false;
    m_pHookFunction = *hookFunc;
    m_strTargetFuncName = *targetFunc;
    // ������תΪ��д�����ڱȶ�
    std::transform(m_strTargetFuncName.begin(), m_strTargetFuncName.end(), m_strTargetFuncName.begin(), ::toupper);

    m_wstrTargetModule = *targetModule;
    // ������תΪ��д�����ڱȶ�
    std::transform(m_wstrTargetModule.begin(), m_wstrTargetModule.end(), m_wstrTargetModule.begin(), ::toupper);

    return true;
}

bool IATHook::Install()
{
    // IATHookʹ��GetModuleHandle��ȡ�ľ����ΪPEͷ�ײ���ַ������HMODULE����ֵ����Ʒ�ʽ
    // ��Ŀ�ľ��Ǳ���ʹ�øõ�ַ����ģ�����ݣ����������;����û�������÷�����HANDLE����
    // ע�⣺GetModuleHandle���صľ����ʵ�ǻ�ַ������Ƶ����������������������ü���
    //      ���ǵ���FreeLibrary��������ü��������Բ�Ҫ�ֶ��ͷ�������
    HMODULE hModule = GetModuleHandle(nullptr); // ��ȡ��ǰ����ģ���ַ
    if (nullptr == hModule)
    {
        Logger::GetInstance().Error(L"GetModuleHandle failed! error = %d", GetLastError());
        return false;
    }

    // PEͷ�ײ���DOSͷ
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;

    PIMAGE_IMPORT_DESCRIPTOR importDescriptor = nullptr;

    // ����������Ҫ����x86��x64
    if (m_bIs64Bit)
    {
        // ��ȡNTͷ
        PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS64)((UINT_PTR)hModule + dosHeader->e_lfanew);

        // ��ȡ�����
        importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((UINT_PTR)hModule +
            ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    }
    else
    {
        PIMAGE_NT_HEADERS32 ntHeader = (PIMAGE_NT_HEADERS32)((UINT_PTR)hModule + dosHeader->e_lfanew);
        importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((UINT_PTR)hModule +
            ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    }

    // ƥ��ģ������
    bool bIsMatch = false;
    do
    {
        std::string dllName = (const char*)((UINT_PTR)hModule + importDescriptor->Name);
        std::wstring wstrDllName = StringUtils::MultiByteToWide(dllName);
        std::transform(wstrDllName.begin(), wstrDllName.end(), wstrDllName.begin(), ::toupper);
        if (wstrDllName != m_wstrTargetModule)
        {
            ++importDescriptor;
            continue;
        }
        bIsMatch = true;
        break;
    } while (importDescriptor);

    if (!bIsMatch)
    {
        Logger::GetInstance().Error(L"Target module %s is not found! ", StringUtils::WideToMultiByte(m_wstrTargetModule).c_str());
        return false;
    }

    if (m_bIs64Bit)
    {
        do
        {
            // ��ȡINT��  ��ź�����Ϣ���纯�����ƣ���ź�����ַ�ĵ�ַ
            PIMAGE_THUNK_DATA64 oriFirstThunk = (PIMAGE_THUNK_DATA64)((UINT_PTR)hModule + importDescriptor->OriginalFirstThunk);

            // ��ȡIAT��  ��ź�����ַ
            PIMAGE_THUNK_DATA64 firstThunk = (PIMAGE_THUNK_DATA64)((UINT_PTR)hModule + importDescriptor->FirstThunk);

            if (oriFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
            {
                // ��ŵ��룬����
                ++oriFirstThunk;
                ++firstThunk;
                continue;
            }

            // ���Ƶ���
            PIMAGE_IMPORT_BY_NAME importName = (PIMAGE_IMPORT_BY_NAME)((UINT_PTR)hModule + oriFirstThunk->u1.AddressOfData);
            std::string strName = importName->Name;
            std::transform(strName.begin(), strName.end(), strName.begin(), ::toupper);
            if (0 != _strcmpi(importName->Name, m_strTargetFuncName.c_str()))
            {
                ++oriFirstThunk;
                ++firstThunk;
                continue;
            }

            // ƥ�䵽��Ӧ������ ��¼ԭ������ַ�����ַ��ԭ������ַ
            m_pOriginalFunction = (void*)firstThunk->u1.Function;

            // �޸��ڴ汣������
            VirtualProtectWrapper virProtect(&firstThunk->u1.Function, sizeof(DWORD_PTR), PAGE_READWRITE);
            if (!virProtect.IsValid())
            {
                Logger::GetInstance().Error(L"VirtualProtect failed! error = %d", GetLastError());
                return false;
            }

            // �滻Ϊ���ǵ�hook����
            firstThunk->u1.Function = (DWORD_PTR)m_pHookFunction;
            return true;
        } while (true);
    }
    else
    {
        do
        {
            // ��ȡINT��  ��ź�����Ϣ���纯�����ƣ���ź�����ַ�ĵ�ַ
            PIMAGE_THUNK_DATA32 oriFirstThunk = (PIMAGE_THUNK_DATA32)((UINT_PTR)hModule + importDescriptor->OriginalFirstThunk);

            // ��ȡIAT��  ��ź�����ַ
            PIMAGE_THUNK_DATA32 firstThunk = (PIMAGE_THUNK_DATA32)((UINT_PTR)hModule + importDescriptor->FirstThunk);

            if (oriFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
            {
                // ��ŵ��룬����
                ++oriFirstThunk;
                ++firstThunk;
                continue;
            }

            // ���Ƶ���
            PIMAGE_IMPORT_BY_NAME importName = (PIMAGE_IMPORT_BY_NAME)((UINT_PTR)hModule + oriFirstThunk->u1.AddressOfData);
            if (0 != _strcmpi(importName->Name, m_strTargetFuncName.c_str()))
            {
                ++oriFirstThunk;
                ++firstThunk;
                continue;
            }

            // ƥ�䵽��Ӧ������ ��¼ԭ������ַ�����ַ��ԭ������ַ
            m_pOriginalFunction = (void*)firstThunk->u1.Function;

            // �޸��ڴ汣������
            VirtualProtectWrapper virProtect(&firstThunk->u1.Function, sizeof(DWORD_PTR), PAGE_READWRITE);
            if (!virProtect.IsValid())
            {
                Logger::GetInstance().Error(L"VirtualProtect failed! error = %d", GetLastError());
                return false;
            }

            // �滻Ϊ���ǵ�hook����
            firstThunk->u1.Function = (DWORD_PTR)m_pHookFunction;
            return true;
        } while (true);
    }
    Logger::GetInstance().Error(L"Target func %s is not found!", m_strTargetFuncName.c_str());
    return false;
}

bool IATHook::Uninstall()
{
    // ж�ز���ֻ��Ҫ��Ŀ�꺯����ַ��Ϊԭ������ַ��Ȼ������ִ��һ��install�Ĳ����Ϳ�����
    m_pHookFunction = m_pOriginalFunction;
    if (false == Install())
    {
        return false;
    }

    m_bIsInstalled = true;
    return true;
}

bool IATHook::IsInstalled() const
{
    return m_bIsInstalled;
}

void* IATHook::GetOriginalFuncAddress() const
{
    return m_pOriginalFunction;
}



