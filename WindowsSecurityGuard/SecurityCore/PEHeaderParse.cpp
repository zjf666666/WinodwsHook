#include "pch.h"
#include "PEHeaderParse.h"

PEHeaderParse::PEHeaderParse(const UINT8* base) : prtAddress(base), arch(Architecture::Unknown)
{
    
}
