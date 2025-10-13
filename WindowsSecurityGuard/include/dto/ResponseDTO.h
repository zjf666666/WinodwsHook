#include "IDTO.h"

class ResponseDTO : public IDTO
{
public:
    ResponseDTO();
    ~ResponseDTO();

    void init() override;
    std::string ToJson() const override;   // DTOת��Ϊjson����
    void FromJson(const std::string& jsonStr) override;  // jsonת��ΪDTO����
};