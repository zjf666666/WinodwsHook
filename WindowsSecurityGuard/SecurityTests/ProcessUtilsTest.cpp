#include "pch.h"
#include "gtest/gtest.h"
#include "ProcessUtils.h"

TEST(ProcessUtils, EnumProcess) {
    // ���Ա�������
    std::vector<DWORD> result = ProcessUtils::EnumProcess();
    EXPECT_GT(result.size(), 30);
}

TEST(ProcessUtils, GetProcessIdByName) {
    // ����ʹ�����ƻ�ȡpid
    DWORD dwPid = ProcessUtils::GetProcessIdByName(L"test_exe.exe");
    EXPECT_EQ(dwPid, 9496);
}

//TEST(ProcessUtils, GetProcessModules1) {
//    // ������ͨ�ַ���ת��
//    std::vector<ModuleInfo> result = ProcessUtils::GetProcessModules(22964);
//    EXPECT_GT(result.size(), 5);
//}