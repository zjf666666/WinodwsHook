#pragma once

#include "InstructionTypes.h"

// ǰ����������͸���ṹ�壬�����ڲ�ʵ��
typedef struct ZydisContext* ZydisContextPtr;

/*
 * zydis���װ�ࣺ��װzydis���ֲ���ʵ��ָ��������ض�����
 * �����ûд�ã��������ʵ����״̬�ģ�����Ӧ����ʹ��ʵ����������
 * ��ZydisContextPtr��Ϊһ��˽�г�Ա������ܹ����ܹ�Ӧ����Ҫ���ģ�
 * ���ݺ�����ַӦ��û�취�жϼܹ�����������ַ���Զ�����ָ�����
 * �ض������󣬸��ݴ����ZydisContextPtr�Ľ����������
 * �������Ը��õ�����ʵ��ϸ�ڣ��ⲿҲ������Ҫ����ZydisContext״̬
 */
class ZydisUtils
{
public:
    static ZydisContextPtr CreateContext(bool is64Bit);

    static void DestroyContext(ZydisContextPtr context);

    /*
     * @brief ʹ��Zydis�����x86/x64ָ���ȡָ��ȡ����ͺ��ض�λ��Ϣ
     * @param [IN] instructionBytes ָ�������ָ���ֽ����е�ָ�룬����Ϊ��
     *        [IN] length ��������
     *        [OUT] parseLength ʵ��ָ���
     *        [OUT] zyContext �洢��������Ľṹ��ָ��
     * @return �����ɹ�����true��ʧ�ܷ���false����ָ����Ч����������ȣ�
     */
    static bool ParseInstruction(
        const void* instructionBytes,
        size_t length,
        size_t* parseLength,  // ʵ�ʽ�������
        ZydisContextPtr zyContext // !!!��ע�⣬�ò������ⲿ�ǲ��ɶ���
    );

    /**
     * @brief �ض�λָ�����ָ�����ͽ�Դ��ַ��ָ���ض�λ��Ŀ���ַ
     *
     * @param [IN] instruction   ָ����Ϣ�ṹ��
     *        [IN] sourceAddress ָ���ԭʼ��ַ
     *        [IN] targetAddress ָ����ض�λ����Ŀ���ַ
     *        [IN] bufferSize    ����������Ĵ�С
     *        [OUT] outputSize   ʵ��д��������������ֽ���
     * @return �ض�λ�ɹ�����true��ʧ�ܷ���false
     */
    static bool RelocateInstruction(
        ZydisContextPtr zyData,
        BYTE* sourceAddress,
        BYTE* targetAddress,
        size_t* bufferSize,
        size_t* outputSize
    );

    static bool IsNeedFrontTrampoline(ZydisContextPtr zyData);

    static bool GenerateFrontTrampoline(
        ZydisContextPtr zyData,
        BYTE* sourceAddress,
        BYTE* targetAddress,
        size_t* bufferSize,
        size_t* outputSize
    );

private:
    // ��ȡָ������
    static InstructionType GetInstructionType(const ZydisContextPtr zyData);

    // ��ȡ�Ƿ������Ѱַ
    static bool IsRelativeInstruction(const ZydisContextPtr zyData);

    // �Ƿ����RIPѰַ
    static bool IsRipInstruction(const ZydisContextPtr zyContext);

    // �ض������תָ��
    static bool RelocateRelativeJump(
        ZydisContextPtr zyData, // ָ��������
        BYTE* sourceAddress,    // ԭʼָ���ַ
        BYTE* targetAddress,    // �޸ĺ�ָ����ַ
        size_t* bufferSize,      // �����������С
        size_t* outputSize      // ʵ��д���С
    );

    // �ض���Զ��ת�����Ե�ַ��ָ�ͨ��ת��Ϊ�����ת
    static bool RelocateAbsoluteJump();

    // �ض�������תָ��
    static bool RelocateIndirectJump();

    // �ض���������תָ��
    static bool RelocateConditionalJump();

    // �ض��������ָ��
    static bool RelocateRelativeCall();

    // �ض���Զ���ã����Ե�ַ��ָ�ͨ��ת��Ϊ��ӵ���
    static bool RelocateAbsoluteCall();

    // �����ض���ֱ�ӿ���
    static bool CopyInstruction(
        ZydisContextPtr zyData,
        BYTE* sourceAddress,
        BYTE* targetAddress,
        size_t* bufferSize,
        size_t* outputSize
    );

    // ���RelativeJump�����Ƿ�Ϸ�
    static bool CheckRelativeJumpParam(
        BYTE* targetAddress,    // �޸ĺ�ָ����ַ
        size_t* bufferSize,      // �����������С
        size_t needSize         // ��Ҫ�Ĵ�С
    );

    // ���RelativeJump�����Ƿ�Ϸ�
    static UINT_PTR CalculateRIPAbsoluteAddr(
        ZydisContextPtr zyData, // ָ��������
        BYTE* sourceAddress    // ԭʼָ���ַ
    );

    // ������Ѱַ��ַ
    static UINT64 CalculateIndirectJumpTarget(
        ZydisContextPtr zyData, // ָ��������
        BYTE* sourceAddress    // ԭʼָ���ַ
    );

    // ptr��ȡ��ַ  len��ȡ����  output�������
    static bool ReadMemory(UINT_PTR ptr, UINT len, UINT64& output);

private:
    //static thread_local uint64_t tlsIndirectTarget; // tls��֤�̰߳�ȫ

private:
    /*
     * ���´���Ϊ�淶�Դ��룬��̬������Ӧ������ʽ���ɶ���ʹ��::����ʽ���е���
     * ɾ���������캯����������ֵ����������Ԫ�������Ա�������п�������
     */
    ZydisUtils() {} // ˽�й��캯������ֹ�ⲿ���ù���
    ~ZydisUtils() {} // ˽��������������ֹ�ⲿ��������
    ZydisUtils(const ZydisUtils&) = delete; // ɾ���������캯��
    ZydisUtils& operator=(const ZydisUtils&) = delete; // ɾ��������ֵ����
};

