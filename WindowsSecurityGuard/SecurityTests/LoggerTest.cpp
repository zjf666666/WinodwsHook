#include "pch.h"
#include "gtest/gtest.h"
#include "Logger.h"
#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>
#include "StringUtils.h"

// 测试程序存在问题，因为logger是单例类，会持续保存文件句柄导致无法删除
class LoggerTest : public ::testing::Test {
protected:
    // 测试前的准备工作
    void SetUp() override {
        // 创建临时测试目录
        testLogDir = L"E:\\WinodwsHook\\TestLogs";
        testLogFile = testLogDir + L"\\test_log.txt";
        
        // 确保测试目录存在
        CreateDirectoryW(testLogDir.c_str(), nullptr);
        
        // 确保测试前日志文件不存在
        DeleteFileW(testLogFile.c_str());
    }

    // 测试后的清理工作
    void TearDown() override {
        // 关闭日志实例
        Logger::GetInstance().Initialize(L"", LogLevel::Info);
        
        // 删除测试文件
        DWORD dwRes;
        if (0 == DeleteFileW(testLogFile.c_str()))
        {
            dwRes = GetLastError();
        }
    }

    // 读取日志文件内容
    std::vector<std::wstring> ReadLogFile() {
        std::vector<std::wstring> lines;
        std::ifstream file(testLogFile, std::ios::binary);
        
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                lines.push_back(StringUtils::MultiByteToWide(line));
            }
            file.close();
        }
        
        return lines;
    }

    std::wstring testLogDir;
    std::wstring testLogFile;
};

// 测试日志文件初始化
TEST_F(LoggerTest, Initialize) {
    // 初始化日志系统
    bool result = Logger::GetInstance().Initialize(testLogFile, LogLevel::Debug);
    
    // 验证初始化成功
    EXPECT_TRUE(result);
    
    // 验证日志文件已创建
    DWORD fileAttrs = GetFileAttributesW(testLogFile.c_str());
    EXPECT_NE(fileAttrs, INVALID_FILE_ATTRIBUTES);
}

// 测试写入日志
TEST_F(LoggerTest, WriteLog) {
    // 初始化日志系统
    Logger::GetInstance().Initialize(testLogFile, LogLevel::Debug);
    
    // 写入不同级别的日志
    Logger::GetInstance().Debug(L"测试Debug日志");
    Logger::GetInstance().Info(L"测试Info日志");
    Logger::GetInstance().Warning(L"测试Warning日志");
    Logger::GetInstance().Error(L"测试Error日志");
    Logger::GetInstance().Fatal(L"测试Fatal日志");
    
    // 关闭日志实例，确保内容写入文件
    Logger::GetInstance().Initialize(L"", LogLevel::Info);
    
    // 读取日志文件内容
    std::vector<std::wstring> logLines = ReadLogFile();
    
    // 验证日志内容
    EXPECT_FALSE(logLines.empty());
    EXPECT_EQ(logLines.size(), 5); // 应该有5行日志
    
    // 验证每行日志包含对应的级别标记
    bool hasDebug = false, hasInfo = false, hasWarning = false, hasError = false, hasFatal = false;
    
    for (const auto& line : logLines) {
        if (line.find(L"DEBUG") != std::string::npos && line.find(L"测试Debug日志") != std::string::npos) {
            hasDebug = true;
        }
        if (line.find(L"INFO") != std::string::npos && line.find(L"测试Info日志") != std::string::npos) {
            hasInfo = true;
        }
        if (line.find(L"WARNING") != std::string::npos && line.find(L"测试Warning日志") != std::string::npos) {
            hasWarning = true;
        }
        if (line.find(L"ERROR") != std::string::npos && line.find(L"测试Error日志") != std::string::npos) {
            hasError = true;
        }
        if (line.find(L"FATAL") != std::string::npos && line.find(L"测试Fatal日志") != std::string::npos) {
            hasFatal = true;
        }
    }
    
    EXPECT_TRUE(hasDebug);
    EXPECT_TRUE(hasInfo);
    EXPECT_TRUE(hasWarning);
    EXPECT_TRUE(hasError);
    EXPECT_TRUE(hasFatal);
}

// 测试日志级别过滤
TEST_F(LoggerTest, LogLevelFiltering) {
    // 初始化日志系统，设置最小级别为Warning
    Logger::GetInstance().Initialize(testLogFile, LogLevel::Warning);
    
    // 写入不同级别的日志
    Logger::GetInstance().Debug(L"测试Debug日志"); // 不应该写入
    Logger::GetInstance().Info(L"测试Info日志");   // 不应该写入
    Logger::GetInstance().Warning(L"测试Warning日志"); // 应该写入
    Logger::GetInstance().Error(L"测试Error日志");   // 应该写入
    Logger::GetInstance().Fatal(L"测试Fatal日志");   // 应该写入
    
    // 关闭日志实例，确保内容写入文件
    Logger::GetInstance().Initialize(L"", LogLevel::Info);
    
    // 读取日志文件内容
    std::vector<std::wstring> logLines = ReadLogFile();
    
    // 验证日志内容
    EXPECT_FALSE(logLines.empty());
    EXPECT_EQ(logLines.size(), 3); // 应该只有3行日志（Warning、Error、Fatal）
    
    // 验证只有Warning及以上级别的日志被写入
    bool hasDebug = false, hasInfo = false, hasWarning = false, hasError = false, hasFatal = false;
    
    for (const auto& line : logLines) {
        if (line.find(L"DEBUG") != std::string::npos) {
            hasDebug = true;
        }
        if (line.find(L"INFO") != std::string::npos) {
            hasInfo = true;
        }
        if (line.find(L"WARNING") != std::string::npos) {
            hasWarning = true;
        }
        if (line.find(L"ERROR") != std::string::npos) {
            hasError = true;
        }
        if (line.find(L"FATAL") != std::string::npos) {
            hasFatal = true;
        }
    }
    
    EXPECT_FALSE(hasDebug); // 不应该有Debug级别的日志
    EXPECT_FALSE(hasInfo);  // 不应该有Info级别的日志
    EXPECT_TRUE(hasWarning);
    EXPECT_TRUE(hasError);
    EXPECT_TRUE(hasFatal);
}

// 测试格式化日志
TEST_F(LoggerTest, FormatLog) {
    // 初始化日志系统
    Logger::GetInstance().Initialize(testLogFile, LogLevel::Info);
    
    // 写入格式化日志
    Logger::GetInstance().Info(L"格式化日志测试 %d, %s", 123, L"测试字符串");
    
    // 关闭日志实例，确保内容写入文件
    Logger::GetInstance().Initialize(L"", LogLevel::Info);
    
    // 读取日志文件内容
    std::vector<std::wstring> logLines = ReadLogFile();
    
    // 验证日志内容
    EXPECT_FALSE(logLines.empty());
    
    // 验证格式化内容正确
    bool hasFormattedContent = false;
    for (const auto& line : logLines) {
        if (line.find(L"格式化日志测试 123, 测试字符串") != std::string::npos) {
            hasFormattedContent = true;
            break;
        }
    }
    
    EXPECT_TRUE(hasFormattedContent);
}