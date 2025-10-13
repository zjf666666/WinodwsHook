#pragma once

// վ��ҵ��Ƕȶ������е�command��command��ҵ��ɸ�֪�ģ���Ҫȷ��ҵ��ɶ���

typedef unsigned int       uint32_t;

// ��Ϣģʽ������/��Ӧ/��־
enum class MessageType : uint32_t
{
    REQUEST,
    RESPONSE,
    EVENT
};

enum class CommandType : uint32_t
{
    // �����ࣨ�ظ�������ʹ����ͬ�������֣�
    OPERATE_FILE = 0x10000,     // �ļ�����
    OPERATE_PROCESS = 0x20000,  // ���̲���
    OPERATE_NETWORK = 0x30000,  // �������

    // ��־��
    EVENT_FILE = 0x100,     // �ļ���־
    EVENT_PROCESS = 0x200,  // ������־
    EVENT_NETWORK = 0x300,  // ������־
};

enum class Command : uint32_t
{
    // ���֧��10���ļ������Ӳ���
    FILE_ADD_MONITOR_CREATE = 0x10000, // ���Ӵ����ļ����
    FILE_ADD_MONITOR_CHANGE,           // �����޸��ļ����
    FILE_ADD_MONITOR_DELETE,           // ����ɾ���ļ����

    FILE_DEL_MONITOR_CREATE = 0x10010, // ���Ӵ����ļ����
    FILE_DEL_MONITOR_CHANGE,           // �����޸��ļ����
    FILE_DEL_MONITOR_DELETE,           // ����ɾ���ļ����

    FILE_QUERY_MONITOR_CREATE = 0x10020, // ���Ӵ����ļ����
    FILE_QUERY_MONITOR_CHANGE,           // �����޸��ļ����
    FILE_QUERY_MONITOR_DELETE,           // ����ɾ���ļ����
};

