#include <cstdlib>
#include <iostream>

#include "core/Application.hpp"

int main(int argc, char **argv)
{
    try
    {
        atmogl::Application app(argc, argv);
        app.run();
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
