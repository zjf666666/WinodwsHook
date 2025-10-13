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
    // 将名称转为大写，便于比对
    std::transform(m_strTargetFuncName.begin(), m_strTargetFuncName.end(), m_strTargetFuncName.begin(), ::toupper);

    m_wstrTargetModule = *targetModule;
    // 将名称转为大写，便于比对
    std::transform(m_wstrTargetModule.begin(), m_wstrTargetModule.end(), m_wstrTargetModule.begin(), ::toupper);

    return true;
}

bool IATHook::Install()
{
    // IATHook使用GetModuleHandle获取的句柄作为PE头首部地址，这是HMODULE返回值的设计方式
    // 其目的就是便于使用该地址访问模块内容，但其他类型句柄并没有这种用法，如HANDLE类型
    // 注意：GetModuleHandle返回的句柄其实是基址这种设计导致这个句柄并不会增加引用计数
    //      但是调用FreeLibrary会减少引用技术，所以不要手动释放这个句柄
    HMODULE hModule = GetModuleHandle(nullptr); // 获取当前进程模块地址
    if (nullptr == hModule)
    {
        Logger::GetInstance().Error(L"GetModuleHandle failed! error = %d", GetLastError());
        return false;
    }

    // PE头首部是DOS头
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;

    PIMAGE_IMPORT_DESCRIPTOR importDescriptor = nullptr;

    // 后续操作需要区分x86和x64
    if (m_bIs64Bit)
    {
        // 获取NT头
        PIMAGE_NT_HEADERS64 ntHeader = (PIMAGE_NT_HEADERS64)((UINT_PTR)hModule + dosHeader->e_lfanew);

        // 获取导入表
        importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((UINT_PTR)hModule +
            ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    }
    else
    {
        PIMAGE_NT_HEADERS32 ntHeader = (PIMAGE_NT_HEADERS32)((UINT_PTR)hModule + dosHeader->e_lfanew);
        importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((UINT_PTR)hModule +
            ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    }

    // 匹配模块名称
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
            // 获取INT表  存放函数信息，如函数名称，存放函数地址的地址
            PIMAGE_THUNK_DATA64 oriFirstThunk = (PIMAGE_THUNK_DATA64)((UINT_PTR)hModule + importDescriptor->OriginalFirstThunk);

            // 获取IAT表  存放函数地址
            PIMAGE_THUNK_DATA64 firstThunk = (PIMAGE_THUNK_DATA64)((UINT_PTR)hModule + importDescriptor->FirstThunk);

            if (oriFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
            {
                // 序号导入，跳过
                ++oriFirstThunk;
                ++firstThunk;
                continue;
            }

            // 名称导入
            PIMAGE_IMPORT_BY_NAME importName = (PIMAGE_IMPORT_BY_NAME)((UINT_PTR)hModule + oriFirstThunk->u1.AddressOfData);
            std::string strName = importName->Name;
            std::transform(strName.begin(), strName.end(), strName.begin(), ::toupper);
            if (0 != _strcmpi(importName->Name, m_strTargetFuncName.c_str()))
            {
                ++oriFirstThunk;
                ++firstThunk;
                continue;
            }

            // 匹配到对应函数了 记录原函数地址保存地址及原函数地址
            m_pOriginalFunction = (void*)firstThunk->u1.Function;

            // 修改内存保护属性
            VirtualProtectWrapper virProtect(&firstThunk->u1.Function, sizeof(DWORD_PTR), PAGE_READWRITE);
            if (!virProtect.IsValid())
            {
                Logger::GetInstance().Error(L"VirtualProtect failed! error = %d", GetLastError());
                return false;
            }

            // 替换为我们的hook函数
            firstThunk->u1.Function = (DWORD_PTR)m_pHookFunction;
            return true;
        } while (true);
    }
    else
    {
        do
        {
            // 获取INT表  存放函数信息，如函数名称，存放函数地址的地址
            PIMAGE_THUNK_DATA32 oriFirstThunk = (PIMAGE_THUNK_DATA32)((UINT_PTR)hModule + importDescriptor->OriginalFirstThunk);

            // 获取IAT表  存放函数地址
            PIMAGE_THUNK_DATA32 firstThunk = (PIMAGE_THUNK_DATA32)((UINT_PTR)hModule + importDescriptor->FirstThunk);

            if (oriFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
            {
                // 序号导入，跳过
                ++oriFirstThunk;
                ++firstThunk;
                continue;
            }

            // 名称导入
            PIMAGE_IMPORT_BY_NAME importName = (PIMAGE_IMPORT_BY_NAME)((UINT_PTR)hModule + oriFirstThunk->u1.AddressOfData);
            if (0 != _strcmpi(importName->Name, m_strTargetFuncName.c_str()))
            {
                ++oriFirstThunk;
                ++firstThunk;
                continue;
            }

            // 匹配到对应函数了 记录原函数地址保存地址及原函数地址
            m_pOriginalFunction = (void*)firstThunk->u1.Function;

            // 修改内存保护属性
            VirtualProtectWrapper virProtect(&firstThunk->u1.Function, sizeof(DWORD_PTR), PAGE_READWRITE);
            if (!virProtect.IsValid())
            {
                Logger::GetInstance().Error(L"VirtualProtect failed! error = %d", GetLastError());
                return false;
            }

            // 替换为我们的hook函数
            firstThunk->u1.Function = (DWORD_PTR)m_pHookFunction;
            return true;
        } while (true);
    }
    Logger::GetInstance().Error(L"Target func %s is not found!", m_strTargetFuncName.c_str());
    return false;
}

bool IATHook::Uninstall()
{
    // 卸载操作只需要把目标函数地址改为原函数地址，然后重新执行一遍install的操作就可以了
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



