#pragma once

/*
 * ��Ϣ����ӿ�
 * ���handle���ǿ��������Ϣͷ�����⣬
 * ����cmd��cmdtype��messageid��requestid�Ȳ�����Ҫ����Ӧ�а���
 * �⵼��handle���������Ҫ������Щ��Ϣ��һ����Ϣͷ��������µı����ֶ�
 * handle��������Ҫ�޸ģ�Υ���˿���ԭ��
 */

// ���࣬����������ҵ����Ϣϸ��
class IBussinessHeader
{

};

class IMessage
{
public:
    virtual ~IMessage() = default;
    virtual IBussinessHeader* GetBussinessHeader() const = 0;
};