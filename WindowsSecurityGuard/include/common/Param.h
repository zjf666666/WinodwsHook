#pragma once

/*
 * 定义HookParam类，用于传递Hook创建参数
 * 典型使用者：调用HookEngine的业务模块
 * 依赖：仅依赖hook_types.h
 */

#include <string>
#include <unordered_map>

class TypeId
{
public:

    // 获取typeid函数，这里使用了模板编程的一个特性
    // 当传入的T类型不同时，实际上会做一个类似于重载的操作生成一个对应类型的get函数
    // 这意味着不同的T类型会有不同的dummy值，而相同的类型的dummy值永远相同
    template<typename T>
    static TypeId Get()
    {
        static char dummy;
        // {}写法同()写法，C++11引入，目的主要是避免()带来的歧义，例如int x(int());这里的()可以表示初始化也可以理解为函数声明
        return TypeId{ &dummy };
    }

    // 重载==和!=符号，后续强转时需要先判断类型，增加安全性
    bool operator==(const TypeId& other) const
    {
        return addr == other.addr;
    }

    bool operator!=(const TypeId& other) const
    {
        return addr != other.addr;
    }
private:
    const void* addr;

    // get函数中使用了TypeId{ &dummy }的构造方法，这里需要写对应的构造函数
    // C++不允许隐式将指针转换为自定义类型，所以使用explicit关键字，确保传入类型为指针
    explicit TypeId(const void* address) : addr(address) {}
};

class Param
{
public:
    // 这里需要操作mapInfo，每个类单独有自己的一份，所以不使用静态方法
    template<typename T>
    void Set(const std::string& strKey, const T& value)
    {
        TypeId typeId = TypeId::Get<T>();
        void* data = new T(value);
        mapInfo.insert_or_assign(strKey, std::make_tuple(typeId, data, &TypeSpecificDeleter<T>));
        //mapInfo[strKey] = std::make_pair<TypeId, void*>(TypeId::Get<T>(), new T(value));
    }

    template<typename T>
    const T* Get(const std::string& strKey) const
    {
        // 没有找到这个值，返回空指针
        auto iter = mapInfo.find(strKey);
        if (iter == mapInfo.end())
        {
            return nullptr;
        }

        const TypeId& id = std::get<0>(iter->second);  // 获取类型标识
        const void* value = std::get<1>(iter->second); // 获取值指针
        // 数据类型不一致，返回空指针
        if (id != TypeId::Get<T>())
        {
            return nullptr;
        }

        return static_cast<const T*>(value);
    }

    ~Param()
    {
        for (auto it = mapInfo.begin(); it != mapInfo.end(); ++it)
        {
            // it->first 是key（参数名），it->second 是pair（包含TypeId和数据指针）
            void* data = std::get<1>(it->second);
            auto& deleter = std::get<2>(it->second);
            deleter(data);
        }
    }

private:
    using DeleterFunc = void(*)(void*);

    template<typename T>
    static void TypeSpecificDeleter(void* ptr) {
        delete static_cast<T*>(ptr);
    }

private:
    // 数据存储表，key:   参数名称，如pid、path、name等
    //            value: TypeId，数据类型，void*指向实际值的指针
    //            值使用void*是因为map只能存储同一类型的值，只能使用void*适配所有类型
    //            delete void* 是一种未定义的行为，需要添加删除器
    // 如传入pid为10的数据，key就是pid，TypeId调用get方法获取（int类型的地址），void*地址内的值就是10，std::function就是delete(int)
    std::unordered_map<std::string, std::tuple<TypeId, void*, DeleterFunc>> mapInfo;
};

