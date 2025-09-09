#include "pch.h"
#include "gtest/gtest.h"
#include "StringUtils.h"

TEST(StringUtilsTest, WideToMultiByte) {
    // 测试普通字符串转换
    std::wstring wide = L"Hello World";
    std::string result = StringUtils::WideToMultiByte(wide);
    EXPECT_EQ(result, "Hello World");
}

TEST(StringUtilsTest, MultiByteToWide) {
    // 测试普通字符串转换
    std::string wide = "Hello World";
    std::wstring result = StringUtils::MultiByteToWide(wide);
    EXPECT_EQ(result, L"Hello World");
}

TEST(StringUtilsTest, FormatString) {
    // 测试普通字符串转换
    std::wstring result = StringUtils::FormatString(L"Hello World %d, %s", 123, L"niu de");
    EXPECT_EQ(result, L"Hello World 123, niu de");
}

TEST(StringUtilsTest, SplitString) {
    // 测试普通字符串转换
    std::vector<std::wstring> result = StringUtils::SplitString(L"郑 俊 峰", L' ');
    std::vector<std::wstring> expected = { L"郑", L"俊", L"峰" };
    EXPECT_TRUE(result == expected);
}

TEST(StringUtilsTest, NormalizePath) {
    // 测试普通字符串转换
    std::wstring result = StringUtils::NormalizePath(L"C:\\郑俊峰/ad/../41341/./");
    EXPECT_EQ(result, L"C:\\郑俊峰\\41341");
}