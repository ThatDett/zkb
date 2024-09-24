#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <string>

#include "CommandHandler.hpp"
#include "Directory.hpp"
#include "Utils.hpp"

namespace fs = std::filesystem;
using    Dir = zkb::Directory;

void 
CommandHandler::WrongUsage(Command command, bool crash /* = false*/) const
{
    std::cerr << "Wrong usage\n";
    if (crash) exit(EXIT_FAILURE);
}

CommandHandler::CommandHandler()
{
    std::string command;
    
    std::cout << fs::current_path().string() << "> ";
    while (std::getline(std::cin, command))
    {
        command += ' ';
        size_t pos     = command.find(' ');
        size_t wordBeg = 0;

        //Splitting
        arg.c = 0;
        for (; pos != std::string::npos; arg.c += 1)
        {
            if (arg.c > CommandHandler::MAX_ARGS)
            {
                std::cerr << "Too many args. argc = " << arg.c << "\n";
                exit(EXIT_FAILURE);
            }
            
            const auto& subStr = command.substr(wordBeg, pos - wordBeg);
            arg.v.at(arg.c) = subStr;
            wordBeg = pos + 1;

            auto nextPos = command.find(' ', wordBeg);
            pos = nextPos;
        }

        std::cerr << arg.v.at(2) << '\n';
        Handle();
    }
}

void 
CommandHandler::Handle()
{
    auto& commandStr = arg.v.at(0);
    if (commandStr == "quit" || commandStr == "q") exit(EXIT_SUCCESS);

    Command command = Command::None;
    if (arg.c == 0) { WrongUsage(command); }

    for (char& c : commandStr) { c = std::tolower(c); }

    if (commandStr == "l")
    {
        HandleNewLine();
    }
    else if (commandStr == "r")
    {
        HandleLineDelete();
    }
    else if (commandStr == "b")
    {
        assert(false && "Not implemented");
    }
    else if (commandStr == "ls")
    {
        ListCurrentDirectory();
    }
    else if (commandStr == "cd")
    {
        ChangeDirectory(arg.v.at(1));
    }
    else
    {
        WrongUsage(Command::None);
    }

    //for (int i = 0; i < arg.c; i += 1) arg.v.at(i) = "null";
    std::cout << fs::current_path().string() << "> ";
}

void 
CommandHandler::HandleNewLine()
{
    std::string lineNumberStr;

    switch (arg.c)
    {
        case 2:
        {
            uint64_t    numberOfLines = Dir::GetNumberOfDirs();
            lineNumberStr = std::to_string(numberOfLines + 1) + " ";
            std::cout << "lineNumber: " << numberOfLines << '\n';
            Dir::CreateDirectory(arg.v.at(1).insert(0, lineNumberStr));
        } break;
        case 3:
        {
            if (!zkb::IsInteger(arg.v.at(2)))
            {
                std::cerr << "Not passing an integer for line number\n";
                exit(EXIT_FAILURE);
            }

            for (auto& elem : Dir::PathIterator())
            {
                auto lineNumber = Dir::GetDirectoryLineNumber(elem.path());

                if (lineNumber >= std::stoll(arg.v.at(2)))
                {
                    Dir::ChangeDirectoryLineNumber(elem, lineNumber + 1);
                }
            }

            Dir::CreateDirectory(arg.v.at(1).insert(0, arg.v.at(2) += ' '));
        } break;
        default: WrongUsage(Command::Line); break;
    }
}

void 
CommandHandler::HandleLineDelete()
{
    fs::directory_entry dir;
    switch (arg.c)
    {
        case 1:
        {
            dir = Dir::DirectoryInLine(Dir::GetNumberOfDirs()); 
        } break;
        case 2:
        {
            if (!zkb::IsInteger(arg.v.at(1)))
            {
                std::cerr << "Not passing an integer for line number\n";
                exit(EXIT_FAILURE);
            }

            dir = Dir::DirectoryInLine(std::stoll(arg.v.at(1))); 
        } break;
        default: WrongUsage(Command::Remove); break;
    }
    
    Dir::RecursivelyDelete(dir);
}

void 
CommandHandler::ListCurrentDirectory()
{
    std::cout << '\n';
    for (const auto& dir : Dir::PathIterator())
    {
        std::cout << "\t\t\t\t" << dir.path().filename().string() << '\n';
    }
    std::cout << '\n';
}

void
CommandHandler::ChangeDirectory(const fs::path& path)
{
    fs::current_path(path);
}

