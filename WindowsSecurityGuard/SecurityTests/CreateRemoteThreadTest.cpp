#include "pch.h"
#include "gtest/gtest.h"
#include "InjectionStrategies.h"

TEST(CreateRemoteThreadStrategy, Inject) {
    CreateRemoteThreadStrategy remotethread;
    bool bRes = remotethread.Inject(21060, L"C:\\Users\\15013\\Desktop\\MyInlineHook.dll");
    EXPECT_EQ(bRes, true);
}