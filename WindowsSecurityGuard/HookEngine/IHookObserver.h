#pragma once

#include <string>

#include "HookCommon.h"

// hook״̬��־
enum class HookStatus
{
    // ����״̬����HookEngine�ڲ�ʹ�ã��û������޸���ֵ��ɾ��
    Installing = 1,    // ���ڰ�װ��Ԥ���������첽��װ������
    Installed = 2,     // ��װ�ɹ�
    InstallFailed = 3, // ��װʧ��
    Uninstalling = 4,  // ����ж�أ�Ԥ����
    Uninstalled = 5,   // ж�سɹ�
    UninstallFailed = 6, // ж��ʧ��

    // ��չ�����û����ڴ˴�����Զ���״̬��ֵ��100��ʼ�������ͻ��
    UserDefinedStart = 100,
    // ���磺UserHookTriggered = 101���û��Զ����Hook����״̬��
};

// Hook�¼�
struct HookEvent {
    // timestamp��ʾ��ʱ�䡱
    // ��������+����pid+Ŀ��ģ��+Ŀ�꺯��+�߳�id���ɡ��ص㡱
    // �����������ҵ����dll���и�ֵ��user�ֶΣ�
    // ���¼�����dll��¼HOOK��������������Ϊ�¼���userData�ֶΣ�
    // ʲô�� �� ʲô�ص� ����ʲô��

    // 1. HookEngine���븳ֵ���ֶ�
    uint64_t timestamp;       // �¼�ʱ��������뼶����HookEngine��ϵͳʱ�����ɣ�
    std::string targetFunc;   // Ŀ�꺯������
    std::string targetModule; // Ŀ��ģ������
    std::string ProcessName;  // ��������
    uint32_t processId;       // Ŀ�����ID
    uint32_t threadId;        // �����¼����߳�ID �����ж��ĸ��߳�Ƶ��������hook
    int errorCode;            // �����루0��ʾ�ɹ���
    std::string errorMsg;     // ������������"Ȩ�޲���"��

    // 2. ��DLL���û������ֶΣ�HookEngine�����𴫵ݣ���������
    std::string user;         // �����ˣ���Service���û�������DLL��ע��ʱ���룩
    void* userData;           // �Զ���ҵ�����ݣ�DLL�ɴ��ָ�룬ʹ��ʱ���н�����
};

class IHookObserver
{
public:
    virtual ~IHookObserver() = default;

    /**
     * @brief Hook �¼�֪ͨ
     * @param [IN] event �¼�����
     *        [IN] hookInfo Hook ��Ϣ
     * @note �� HookEngine ����
     */
    virtual void OnHookEvent(HookStatus event, const HookEvent& hookInfo) = 0;
};