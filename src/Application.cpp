#include <cstdlib>
#include <iostream>
#include <string>
#include <array>

#include "Application.hpp"
#include "CommandHandler.hpp"

namespace fs = std::filesystem;
Application::Application(char** _argv, int _argc) :
    argv(_argv), argc(_argc)
{
    if (argc == 1)
    {
        RunTooling();
    }
}

void Application::RunTooling()
{
    CommandHandler handler;
}

void Application::Build()
{

}
