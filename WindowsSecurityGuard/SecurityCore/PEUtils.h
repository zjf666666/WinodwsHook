#pragma once

#include <string>
#include <vector>

#include <Windows.h>

/* PEͷ���������� */
class PEUtils {
public:
    // �ܹ����
    static bool Is64BitPE(const std::wstring& filePath);
    static bool Is64BitPE(void* moduleBase);
    static bool IsSameMachineType(void* moduleA, void* moduleB);

    // ��������
    static bool HasExecutableSection(void* moduleBase);
    static bool IsAddressInExecutableSection(void* moduleBase, void* address);

    // �����/���������
    static bool GetImportedFunctions(void* moduleBase, std::vector<std::string>& functions);
    static bool GetExportedFunctions(void* moduleBase, std::vector<std::string>& functions);

    // ����PE��Ϣ
    static DWORD GetEntryPoint(void* moduleBase);
    static bool IsManaged(void* moduleBase);  // ���.NET����
    static bool IsSigned(void* moduleBase);   // �������ǩ��

private:
    // ��������
    static PIMAGE_NT_HEADERS GetNTHeaders(void* moduleBase);
    static PIMAGE_SECTION_HEADER GetSectionHeader(void* moduleBase, const char* sectionName);

private:
    /*
     * ���´���Ϊ�淶�Դ��룬������Ӧ������ʽ���ɶ���ʹ��::����ʽ���е���
     * ɾ���������캯����������ֵ����������Ԫ�������Ա�������п�������
     */
    PEUtils() = delete;
    ~PEUtils() = delete;
    PEUtils(const PEUtils&) = delete;
    PEUtils& operator=(const PEUtils&) = delete;
};

