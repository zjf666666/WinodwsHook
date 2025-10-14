/*
 * ҵ��ģ�飬��Ҫ����
 * 1. �����ļ������̡�ע������������
 * 2. ����������Hook���ص���Ϣ�������ظ�ͨ�Ų�
 * 
 * ���ģʽ��
 * 1. ProtectionModules: ����ģʽ��ҵ��ӿڣ�����ҵ����ֻ�ܵ������ģ��ӿ�
 * 2. IProtectionHandle: ����ӿ�
 * 3. ProtectionFactory: ������������ϳ���ӿڣ������ļ������̡�ע�������ȴ�����ϸ��
 *
 * ʹ�÷�ʽ��
 * ���ṩһ���������� RegisterModule ���øú�������ģ����Զ����Լ�֧�ֵĲ������ͼ���Ӧ�Ĵ��������ظ����÷�
 */

#include "pch.h"

#include "ProtectionModules.h"
#include "../SecurityService/CommandRegistry.h"

 // ������������ȡ֧������
extern "C" __declspec(dllexport)
void RegisterModule(CommandRegistry* registry)
{
    registry->RegisterHandler(CommandType::OPERATE_FILE, ProtectionModules::GetInstance().Handle);
    registry->RegisterHandler(CommandType::OPERATE_PROCESS, ProtectionModules::GetInstance().Handle);
    // ...
}