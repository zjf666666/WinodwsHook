#include "pch.h"
#include "gtest/gtest.h"
#include "InjectionStrategies.h"

TEST(CreateRemoteThreadStrategy, Inject) {
    CreateRemoteThreadStrategy remotethread;
    bool bRes = remotethread.Inject(16372, L"E:\\WinodwsHook\\WindowsSecurityGuard\\x64\\Debug\\IATHookDll.dll");
    EXPECT_EQ(bRes, true);
}