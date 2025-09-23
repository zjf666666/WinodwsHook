#include "pch.h"
#include "gtest/gtest.h"
#include "ProcessInjectionManager.h"

TEST(ProcessInjectionManager, InjectDll) {
    //bool result = ProcessInjectionManager::GetInstance().InjectDll(21040, L"C:\\Users\\15013\\Desktop\\×¢Èë²âÊÔÎÄ¼þ\\MyInlineHook.dll");
    bool result = ProcessInjectionManager::GetInstance().InjectDll(20440, L"E:\\WinodwsHook\\WindowsSecurityGuard\\x64\\Debug\\FSProtect.dll");
    EXPECT_EQ(result, true);
}