#pragma once

/*
 * ����HookParam�࣬���ڴ���Hook��������
 * ����ʹ���ߣ�����HookEngine��ҵ��ģ��
 * ������������hook_types.h
 */

#include <string>
#include <unordered_map>

class TypeId
{
public:

    // ��ȡtypeid����������ʹ����ģ���̵�һ������
    // �������T���Ͳ�ͬʱ��ʵ���ϻ���һ�����������صĲ�������һ����Ӧ���͵�get����
    // ����ζ�Ų�ͬ��T���ͻ��в�ͬ��dummyֵ������ͬ�����͵�dummyֵ��Զ��ͬ
    template<typename T>
    static TypeId Get()
    {
        static char dummy;
        // {}д��ͬ()д����C++11���룬Ŀ����Ҫ�Ǳ���()���������壬����int x(int());�����()���Ա�ʾ��ʼ��Ҳ�������Ϊ��������
        return TypeId{ &dummy };
    }

    // ����==��!=���ţ�����ǿתʱ��Ҫ���ж����ͣ����Ӱ�ȫ��
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

    // get������ʹ����TypeId{ &dummy }�Ĺ��췽����������Ҫд��Ӧ�Ĺ��캯��
    // C++��������ʽ��ָ��ת��Ϊ�Զ������ͣ�����ʹ��explicit�ؼ��֣�ȷ����������Ϊָ��
    explicit TypeId(const void* address) : addr(address) {}
};

class Param
{
public:
    // ������Ҫ����mapInfo��ÿ���൥�����Լ���һ�ݣ����Բ�ʹ�þ�̬����
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
        // û���ҵ����ֵ�����ؿ�ָ��
        auto iter = mapInfo.find(strKey);
        if (iter == mapInfo.end())
        {
            return nullptr;
        }

        const TypeId& id = std::get<0>(iter->second);  // ��ȡ���ͱ�ʶ
        const void* value = std::get<1>(iter->second); // ��ȡֵָ��
        // �������Ͳ�һ�£����ؿ�ָ��
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
            // it->first ��key������������it->second ��pair������TypeId������ָ�룩
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
    // ���ݴ洢��key:   �������ƣ���pid��path��name��
    //            value: TypeId���������ͣ�void*ָ��ʵ��ֵ��ָ��
    //            ֵʹ��void*����Ϊmapֻ�ܴ洢ͬһ���͵�ֵ��ֻ��ʹ��void*������������
    //            delete void* ��һ��δ�������Ϊ����Ҫ���ɾ����
    // �紫��pidΪ10�����ݣ�key����pid��TypeId����get������ȡ��int���͵ĵ�ַ����void*��ַ�ڵ�ֵ����10��std::function����delete(int)
    std::unordered_map<std::string, std::tuple<TypeId, void*, DeleterFunc>> mapInfo;
};

