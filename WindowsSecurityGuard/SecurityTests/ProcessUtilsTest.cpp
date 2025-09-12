#include "pch.h"
#include "gtest/gtest.h"
#include "ProcessUtils.h"

TEST(ProcessUtils, EnumProcess) {
    // 测试遍历进程
    std::vector<DWORD> result = ProcessUtils::EnumProcess();
    EXPECT_GT(result.size(), 30);
}

TEST(ProcessUtils, GetProcessIdByName) {
    // 测试使用名称获取pid
    DWORD dwPid = ProcessUtils::GetProcessIdByName(L"test_exe.exe");
    EXPECT_EQ(dwPid, 9496);
}

//TEST(ProcessUtils, GetProcessModules1) {
//    // 测试普通字符串转换
//    std::vector<ModuleInfo> result = ProcessUtils::GetProcessModules(22964);
//    EXPECT_GT(result.size(), 5);
//}