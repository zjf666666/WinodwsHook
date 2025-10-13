#pragma once

#include <string>
#include <unordered_map>

// ������Ķ��������ǰ����û�����������ǰ���������׼��λ����
// ����ϸ�ڲ�Ӧ�ù��౩¶������ע����ΪHook���̵�һ���֣��û����޸�֪�ģ������һ������С�ף��޷�������������ν��
// ��Ҫ������Ĵ���ϸ�ڣ���������ΪȨ�޲��㵼�µĻ���������һЩԭ��
enum class ErrorCode : uint32_t
{
    SUCCESS = 0,

    // TODO: ������дһ��ע��ʧ����Ϊdemo���øĳɸ������
    INJECT_FAILED = 0x1000
};

enum class Language : uint8_t
{
    CHINESE,
    ENGLISG
};

class ErrorManager
{
public:
    // ��ȡ������Ϣ
    static std::string GetErrorMessage(ErrorCode error);
    static std::string GetErrorMessage(ErrorCode error, Language language);

    static void SetLanguage(Language language);

private:
    static const std::unordered_map<ErrorCode, std::unordered_map<Language, std::string>> umapError;
    static Language lan;
};