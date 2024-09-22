#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <filesystem>

#include "CommandHandler.hpp"

namespace fs = std::filesystem;

void CommandHandler::WrongUsage(Command command)
{
    std::cout << "Wrong usage\n";
    exit(EXIT_FAILURE);
}


CommandHandler::CommandHandler(char** _argv, int _argc) :
    argc(_argc)
{
    Command command = Command::None;
    if (argc == 1) { WrongUsage(command); }

    argv.reserve(argc);
    for (int i = 0; i < argc; ++i)
    {
        argv.emplace_back(_argv[i]);
    }

    for (const auto& str : argv)
    {
        std::cout << str << std::endl;
    }


    std::string commandStr(argv[1]);
    for (char& c : commandStr) { c = std::tolower(c); }
    if (commandStr == "l")
    {
        HandleNewLine();
    }
    else if (commandStr == "r")
    {
        HandleLineDelete();
    }
    else if (commandStr == "build")
    {
        assert(false && "Not implemented");
    }
    else
    {
        WrongUsage(Command::None);
    }
}

void CommandHandler::UpdateNumberOfDirs()
{
    numberOfDirs = 0;
    for (const auto& throwaway : fs::directory_iterator(fs::current_path()))
    {
        numberOfDirs += 1;            
    }
}

void CommandHandler::HandleNewLine()
{
    char lineNumberChar;
    UpdateNumberOfDirs();

    {size_t i = 0;
    for (const auto& dir : dirIt)
    {
        std::cout << dir << '\n';
       
        //TODO: Parse line number correctly and convert to uint
        if (i == numberOfDirs - 1)
        {
            const auto& str = dir.path().filename().string();
            lineNumberChar = str.at(0);
            lineNumber = lineNumberChar - '0';
            std::cout << "lineNumber: " << lineNumber << '\n';
        }
        i += 1; 
    }}

    lineNumberChar += 1;
    fs::create_directory(argv[2].insert(0, 1, lineNumberChar).insert(1, 1, ' '));
}

void CommandHandler::HandleLineDelete()
{
    int lineNum = argv[2].at(0) - '0';
    for (const auto& dir : dirIt)
    {
        if (dir.path().filename().string().at(0) == argv.at(2).at(0))
        {
            fs::remove(dir.path());
        }
    }
}
