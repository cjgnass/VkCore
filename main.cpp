#include <iostream>
#include "include/app.hpp"

int main()
{
    try
    {
        App app;
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
}