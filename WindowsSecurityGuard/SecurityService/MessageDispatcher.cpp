//#include "pch.h"
//#include "MessageDispatcher.h"
//
//#include "Message.h"
//#include "../SecurityCore/ProcessUtils.h"
//#include "../SecurityCore/ProcessInjectionManager.h"
//#include "../include/dto/IDTO.h"
//
//WindowsSecurityGuard::Message MessageDispatcher::Dispatch(const WindowsSecurityGuard::Message& request)
//{
//    switch ((WindowsSecurityGuard::MessageType)request.header.type)
//    {
//    case WindowsSecurityGuard::MessageType::InjectHook:
//        return HandleProcessFileMointor(request);
//        break;
//    default:
//        break;
//    }
//}
//
//WindowsSecurityGuard::Message MessageDispatcher::HandleProcessFileMointor(const WindowsSecurityGuard::Message& request)
//{
//    // 解析数据成dto格式
//    CreateFileMonitorDTO dto;
//    dto.FromJson(std::string(request.body.begin(), request.body.end()));
//    std::wstring wstrDir = ProcessUtils::GetCurrentProcessDir();
//    auto pid = dto.param.Get<int>("request_process_id");
//    if (ProcessInjectionManager::GetInstance().InjectDll(*pid, wstrDir + std::wstring(L"IATHOOKDLL.dll")))
//    {
//        return WindowsSecurityGuard::ProtocolUtils::MakeResponse((WindowsSecurityGuard::MessageType)request.header.type, request.header.correlationId);
//    }
//    return WindowsSecurityGuard::ProtocolUtils::MakeError(request.header.correlationId, 0, "InjectDll failed");
//}
