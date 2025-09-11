#pragma once

#include <string>

/* Hook 接口类，定义所有 Hook 实现的通用接口 */
class IHook
{
public:
    ~IHook() = default; // 更符合现代C++风格习惯 等同于 ~IHook() {}

    // 安装 Hook
    virtual bool Install() = 0;

    // 卸载 Hook
    virtual bool Uninstall() = 0;

    // 检查 Hook 是否已安装
    virtual bool IsInstalled() const = 0;

    // 获取原始函数
    template<typename T>
    T GetOriginalFunction() {
        return reinterpret_cast<T>(GetOriginalFunctionAddress());
    }

    // 获取目标模块名
    // 这里返回值是wstring，因为模块的路径名称（DLL或exe）通常需要支持Unicode字符
    // 例如LoadLibraryW，GetModuleHandleW等
    virtual const std::wstring& GetTargetModule() const = 0;

    // 获取目标函数名
    // 这里返回值是string，因为函数名称在windows中一般是由ASCII码存放的，在PE头中，使用的是ASCII码而非Unicode字符
    // 此外GetProcAddress函数只支持string传入，所以这里返回string
    virtual const std::string& GetTargetFunction() const = 0;

    // 获取 Hook 类型
    virtual const std::wstring& GetHookType() const = 0;

protected:
    // 获取原始函数地址
    virtual void* GetOriginalFunctionAddress() const = 0;
};

