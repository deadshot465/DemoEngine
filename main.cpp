#include "WindowDX.h"
#include <iostream>

int main()
{
    auto window = DX::Window(1024, 768, "Spring Vacation Osarai");

    std::cout << "Select an API: \n";
    std::cout << "\t1) DirectX 11\n";
    std::cout << "\t2) DirectX 12\n";

    int choice = 0;
    std::cin >> choice;

    switch (choice)
    {
    case 1:
        if (window.Initialize(false))
            window.Broadcast();
        break;
    case 2:
        if (window.Initialize(true))
            window.Broadcast();
        break;
    default:
        break;
    }

    return 0;
}
