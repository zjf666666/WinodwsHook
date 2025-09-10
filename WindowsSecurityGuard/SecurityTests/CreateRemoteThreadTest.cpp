#include "pch.h"
#include "gtest/gtest.h"
#include "InjectionStrategies.h"

TEST(CreateRemoteThreadStrategy, Inject) {
    CreateRemoteThreadStrategy remotethread;
    bool bRes = remotethread.Inject(1, L"23");
    EXPECT_EQ(bRes, true);
}