#pragma once

#include <string>
#include <vector>

#include <Windows.h>

enum class Architecture {
    X86,
    X64,
    Unknown
};

/* 
 * PEͷ����������
 * ��ϣ���������Դ���ÿ����ľ������ݣ���NTͷ��DOSͷ�������������
 * ����Щ���ݵĻ�ȡ������ǰ��Ľ�������ģ����ÿ������֮�����������ϵ
 * ���������״̬�ģ�����������Ϊʵ�������࣬����PEͷ�ṹ����Ϊ˽�г�Ա
 * ����ʱ����PE�ļ��ײ���ַ�����Զ����ڲ���������״̬
 */
class PEHeaderParse {
public:
    PEHeaderParse(const UINT8* base);

private:
    // ��������
    bool GetNTHeaders(void* moduleBase);
    bool GetSectionHeader(void* moduleBase, const char* sectionName);

private:
    const UINT8* prtAddress;
    Architecture arch; // �ܹ���Ϣ
    IMAGE_DOS_HEADER dosHeader; // DOSͷ

    IMAGE_NT_HEADERS32 ntHeader32; // 32λNTͷ
    IMAGE_NT_HEADERS64 ntHeader64; // 64λNTͷ

    IMAGE_FILE_HEADER fileHeader; // �ļ�ͷ

    IMAGE_OPTIONAL_HEADER32 opHeader32; // 32λ��ѡͷ
    IMAGE_OPTIONAL_HEADER64 opHeader64; // 64λ��ѡͷ

    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

