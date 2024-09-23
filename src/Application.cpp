#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <array>

#include "Application.hpp"
#include "CommandHandler.hpp"

namespace fs = std::filesystem;
Application::Application(char** _argv, int _argc) :
    argv(_argv), argc(_argc)
{
}

void Application::Run()
{
    std::string command;
    CommandHandler::ArgvT argv;
    CommandHandler commandHandler(argv);
    
    std::cout << fs::current_path().string() << "> ";
    while (std::getline(std::cin, command))
    {
        command += ' ';
        auto          pos     = command.find(' ');
        decltype(pos) wordBeg = 0;

        //Splitting
        int argc = 0;
        for (; pos != std::string::npos; argc += 1)
        {
            if (argc >= CommandHandler::MAX_ARGS - 1)
            {
                std::cerr << "Too many args. argc = " << argc << "\n";
                exit(EXIT_FAILURE);
            }
            
            const auto& subStr = command.substr(wordBeg, pos - wordBeg);
            argv.at(argc) = subStr;
            wordBeg = pos + 1;

            auto nextPos = command.find(' ', wordBeg);
            pos = nextPos;
        }

        commandHandler.Handle(argc);
    }
}
