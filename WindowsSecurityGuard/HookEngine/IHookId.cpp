#include "pch.h"
#include "IHookId.h"

std::string IHookId::ReplaceSpecChar(const std::string& str)
{
    std::string strRes = "";
    for (auto c : str)
    {
        switch (c)
        {
        case '|':  // 分隔符
            strRes += "%7C";
            break;
        case '=':  // 键值分隔符
            strRes += "%3D";
            break;
        case '%':  // 转义符本身
            strRes += "%25";
            break;
        default:   // 其他字符不转义
            strRes += c;
            break;
        }
    }
    return strRes;
}
