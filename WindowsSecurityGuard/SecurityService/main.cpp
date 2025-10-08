#include "pch.h"

#include "SecurityService.h"

int main()
{
    SecurityService ss;
    ss.Initialize();
    ss.Start();
    while (1)
    {
        Sleep(1000);
    }
    return 1;
}