#include "IDTO.h"

class ResponseDTO : public IDTO
{
public:
    ResponseDTO();
    ~ResponseDTO();

    void init() override;
    std::string ToJson() const override;   // DTO转化为json函数
    void FromJson(const std::string& jsonStr) override;  // json转化为DTO函数
};