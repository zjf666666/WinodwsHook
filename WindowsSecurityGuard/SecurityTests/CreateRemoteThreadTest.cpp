#include "pch.h"
#include "gtest/gtest.h"
#include "InjectionStrategies.h"

TEST(CreateRemoteThreadStrategy, Inject) {
    CreateRemoteThreadStrategy remotethread;
    bool bRes = remotethread.Inject(10400, L"C:\\Users\\15013\\Desktop\\×¢Èë²âÊÔÎÄ¼ş\\MyInlineHook.dll");
    EXPECT_EQ(bRes, true);
}