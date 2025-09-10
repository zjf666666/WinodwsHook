#include "pch.h"
#include "gtest/gtest.h"
#include "ProcessUtils.h"

TEST(ProcessUtils, EnumProcess) {
    // ������ͨ�ַ���ת��
    std::vector<DWORD> result = ProcessUtils::EnumProcess();
    EXPECT_GT(result.size(), 30);
}

TEST(ProcessUtils, GetProcessInfo) {
    // ������ͨ�ַ���ת��
    ProcessInfo result = ProcessUtils::GetProcessInfo(22964);
    EXPECT_EQ(result.processName, L"test_exe.exe");
}

TEST(ProcessUtils, GetProcessModules1) {
    // ������ͨ�ַ���ת��
    std::vector<ModuleInfo> result = ProcessUtils::GetProcessModules(22964);
    EXPECT_GT(result.size(), 5);
}