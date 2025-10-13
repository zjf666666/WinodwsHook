/*
 * 业务模块，主要负责
 * 1. 处理文件、进程、注册表、网络等请求
 * 2. 接收驱动、Hook返回的信息，并返回给通信层
 * 
 * 设计模式：
 * 1. ProtectionModules: 单例模式，业务接口，所有业务处理只能调用这个模块接口
 * 2. IProtectionHandle: 抽象接口
 * 3. ProtectionFactory: 创建工厂，配合抽象接口，隐藏文件、进程、注册表、网络等处理函数细节
 *
 * 使用方式：
 * 提供两个导出函数 GetSupportCmdType、HandleRequest
 * GetSupportCmdType：获取支持的CMD类型
 * HandleRequest：处理函数
 */

#include "pch.h"

#include "ProtectionModules.h"
#include "../SecurityService/CommandRegistry.h"

 // 导出函数，获取支持类型
extern "C" __declspec(dllexport)
void RegisterModule(CommandRegistry* registry)
{
    registry->RegisterHandler(CommandType::OPERATE_FILE, ProtectionModules::GetInstance().Handle);
    registry->RegisterHandler(CommandType::OPERATE_PROCESS, ProtectionModules::GetInstance().Handle);
    // ...
}