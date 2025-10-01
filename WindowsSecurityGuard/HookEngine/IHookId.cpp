#include "pch.h"
#include "IHookId.h"

std::string IHookId::ReplaceSpecChar(const std::string& str)
{
    std::string strRes = "";
    for (auto c : str)
    {
        switch (c)
        {
        case '|':  // �ָ���
            strRes += "%7C";
            break;
        case '=':  // ��ֵ�ָ���
            strRes += "%3D";
            break;
        case '%':  // ת�������
            strRes += "%25";
            break;
        default:   // �����ַ���ת��
            strRes += c;
            break;
        }
    }
    return strRes;
}
