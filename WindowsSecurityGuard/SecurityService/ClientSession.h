#pragma once

#include <Memory>

#include <Windows.h>

struct Message;

class ClientSession : public std::enable_shared_from_this<ClientSession>
{
public:
    // ���캯�������������ӵĹܵ������ַ�������ѡ������־���ȸ�������
    ClientSession(HANDLE pipe, std::shared_ptr<class MessageDispatcher> dispatcher);

    // �����Ự�����̣߳��򽫻Ự�ύ���̳߳أ�����ʼ��дѭ��
    bool Start();

    // �رջỰ��ֹͣ��д���ͷ���Դ���̰߳�ȫ��
    void Close();

    // �����¼���������첽���ͣ������¼���Ϣд��ܵ��������Ƿ��ͳɹ�
    bool SendEvent(const Message& eventMsg);

    // ��ȡ���һ������ʱ��������ڳ�ʱ����
    uint64_t GetLastHeartbeatTs() const;

private:
    // �Ự��ѭ������ȡ��Ϣͷ�븺�أ��������ɷַ���������д����Ӧ
    void RunLoop();

    // ��ȡһ��������Ϣ��������/���������Ƿ�ɹ���
    bool ReadMessage(Message& outMsg, DWORD timeoutMs);

    // д��һ��������Ϣ������ǰ׺д�룬�����Ƿ�ɹ���
    bool WriteMessage(const Message& msg, DWORD timeoutMs);

    // ����������������ʱ�䣬��Ҫʱ�ظ�������Ӧ
    void HandleHeartbeat(const Message& req);
};

