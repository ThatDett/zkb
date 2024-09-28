#include <cstring>
#include <iostream>

#include "Application.hpp"
#include "CommandHandler.hpp"

namespace fs = std::filesystem;
Application::Application(char** _argv, int _argc) :
    argv(_argv), argc(_argc)
{
    if (argc == 1)
    {
        //Run tooling
        CommandHandler();
    }
    else if (std::string(argv[1]) == "build")
    {
        Build();
    }
}

void Application::Build()
{
    std::cerr << "Not implemented.\n";
}
