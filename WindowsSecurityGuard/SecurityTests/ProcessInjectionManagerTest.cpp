#include "pch.h"
#include "gtest/gtest.h"
#include "ProcessInjectionManager.h"

TEST(ProcessInjectionManager, InjectDll) {
    bool result = ProcessInjectionManager::GetInstance().InjectDll(12840, L"C:\\Users\\15013\\Desktop\\ע������ļ�\\MyInlineHook.dll");
    EXPECT_EQ(result, true);
}