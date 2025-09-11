#include "pch.h"
#include "gtest/gtest.h"
#include "Logger.h"
#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>
#include "StringUtils.h"

// ���Գ���������⣬��Ϊlogger�ǵ����࣬����������ļ���������޷�ɾ��
class LoggerTest : public ::testing::Test {
protected:
    // ����ǰ��׼������
    void SetUp() override {
        // ������ʱ����Ŀ¼
        testLogDir = L"E:\\WinodwsHook\\TestLogs";
        testLogFile = testLogDir + L"\\test_log.txt";
        
        // ȷ������Ŀ¼����
        CreateDirectoryW(testLogDir.c_str(), nullptr);
        
        // ȷ������ǰ��־�ļ�������
        DeleteFileW(testLogFile.c_str());
    }

    // ���Ժ��������
    void TearDown() override {
        // �ر���־ʵ��
        Logger::GetInstance().Initialize(L"", LogLevel::Info);
        
        // ɾ�������ļ�
        DWORD dwRes;
        if (0 == DeleteFileW(testLogFile.c_str()))
        {
            dwRes = GetLastError();
        }
    }

    // ��ȡ��־�ļ�����
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

// ������־�ļ���ʼ��
TEST_F(LoggerTest, Initialize) {
    // ��ʼ����־ϵͳ
    bool result = Logger::GetInstance().Initialize(testLogFile, LogLevel::Debug);
    
    // ��֤��ʼ���ɹ�
    EXPECT_TRUE(result);
    
    // ��֤��־�ļ��Ѵ���
    DWORD fileAttrs = GetFileAttributesW(testLogFile.c_str());
    EXPECT_NE(fileAttrs, INVALID_FILE_ATTRIBUTES);
}

// ����д����־
TEST_F(LoggerTest, WriteLog) {
    // ��ʼ����־ϵͳ
    Logger::GetInstance().Initialize(testLogFile, LogLevel::Debug);
    
    // д�벻ͬ�������־
    Logger::GetInstance().Debug(L"����Debug��־");
    Logger::GetInstance().Info(L"����Info��־");
    Logger::GetInstance().Warning(L"����Warning��־");
    Logger::GetInstance().Error(L"����Error��־");
    Logger::GetInstance().Fatal(L"����Fatal��־");
    
    // �ر���־ʵ����ȷ������д���ļ�
    Logger::GetInstance().Initialize(L"", LogLevel::Info);
    
    // ��ȡ��־�ļ�����
    std::vector<std::wstring> logLines = ReadLogFile();
    
    // ��֤��־����
    EXPECT_FALSE(logLines.empty());
    EXPECT_EQ(logLines.size(), 5); // Ӧ����5����־
    
    // ��֤ÿ����־������Ӧ�ļ�����
    bool hasDebug = false, hasInfo = false, hasWarning = false, hasError = false, hasFatal = false;
    
    for (const auto& line : logLines) {
        if (line.find(L"DEBUG") != std::string::npos && line.find(L"����Debug��־") != std::string::npos) {
            hasDebug = true;
        }
        if (line.find(L"INFO") != std::string::npos && line.find(L"����Info��־") != std::string::npos) {
            hasInfo = true;
        }
        if (line.find(L"WARNING") != std::string::npos && line.find(L"����Warning��־") != std::string::npos) {
            hasWarning = true;
        }
        if (line.find(L"ERROR") != std::string::npos && line.find(L"����Error��־") != std::string::npos) {
            hasError = true;
        }
        if (line.find(L"FATAL") != std::string::npos && line.find(L"����Fatal��־") != std::string::npos) {
            hasFatal = true;
        }
    }
    
    EXPECT_TRUE(hasDebug);
    EXPECT_TRUE(hasInfo);
    EXPECT_TRUE(hasWarning);
    EXPECT_TRUE(hasError);
    EXPECT_TRUE(hasFatal);
}

// ������־�������
TEST_F(LoggerTest, LogLevelFiltering) {
    // ��ʼ����־ϵͳ��������С����ΪWarning
    Logger::GetInstance().Initialize(testLogFile, LogLevel::Warning);
    
    // д�벻ͬ�������־
    Logger::GetInstance().Debug(L"����Debug��־"); // ��Ӧ��д��
    Logger::GetInstance().Info(L"����Info��־");   // ��Ӧ��д��
    Logger::GetInstance().Warning(L"����Warning��־"); // Ӧ��д��
    Logger::GetInstance().Error(L"����Error��־");   // Ӧ��д��
    Logger::GetInstance().Fatal(L"����Fatal��־");   // Ӧ��д��
    
    // �ر���־ʵ����ȷ������д���ļ�
    Logger::GetInstance().Initialize(L"", LogLevel::Info);
    
    // ��ȡ��־�ļ�����
    std::vector<std::wstring> logLines = ReadLogFile();
    
    // ��֤��־����
    EXPECT_FALSE(logLines.empty());
    EXPECT_EQ(logLines.size(), 3); // Ӧ��ֻ��3����־��Warning��Error��Fatal��
    
    // ��ֻ֤��Warning�����ϼ������־��д��
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
    
    EXPECT_FALSE(hasDebug); // ��Ӧ����Debug�������־
    EXPECT_FALSE(hasInfo);  // ��Ӧ����Info�������־
    EXPECT_TRUE(hasWarning);
    EXPECT_TRUE(hasError);
    EXPECT_TRUE(hasFatal);
}

// ���Ը�ʽ����־
TEST_F(LoggerTest, FormatLog) {
    // ��ʼ����־ϵͳ
    Logger::GetInstance().Initialize(testLogFile, LogLevel::Info);
    
    // д���ʽ����־
    Logger::GetInstance().Info(L"��ʽ����־���� %d, %s", 123, L"�����ַ���");
    
    // �ر���־ʵ����ȷ������д���ļ�
    Logger::GetInstance().Initialize(L"", LogLevel::Info);
    
    // ��ȡ��־�ļ�����
    std::vector<std::wstring> logLines = ReadLogFile();
    
    // ��֤��־����
    EXPECT_FALSE(logLines.empty());
    
    // ��֤��ʽ��������ȷ
    bool hasFormattedContent = false;
    for (const auto& line : logLines) {
        if (line.find(L"��ʽ����־���� 123, �����ַ���") != std::string::npos) {
            hasFormattedContent = true;
            break;
        }
    }
    
    EXPECT_TRUE(hasFormattedContent);
}