#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    std::cout << "ÇëÊäÈëÊý¾Ý: ";
    std::string strTest = "";
    while (std::cin >> strTest)
    {
        MessageBoxA(NULL, "TEST", "TEST", MB_OK);
    }
    return 1;
}