#include <cstdlib>
#include <iostream>
#include <string>
#include <filesystem>

#include "Application.hpp"

int main(int argc, char** argv)
{
    Application app(argv, argc);
    app.Run();
}
