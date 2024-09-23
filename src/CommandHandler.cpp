#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <filesystem>

#include "CommandHandler.hpp"
#include "Directory.hpp"

namespace fs = std::filesystem;
using    Dir = zkb::Directory;

void 
CommandHandler::WrongUsage(Command command, bool crash /* = false*/)
{
    std::cerr << "Wrong usage\n";
    if (crash) exit(EXIT_FAILURE);
}

CommandHandler::CommandHandler(ArgvT& _argv) : argv(_argv) {}

void 
CommandHandler::Handle(int _argc)
{
    argc = _argc;
    auto& commandStr = argv.at(0);
    if (commandStr == "quit" || commandStr == "q") exit(EXIT_SUCCESS);

    Command command = Command::None;
    if (argc == 0) { WrongUsage(command); }

    for (char& c : commandStr) { c = std::tolower(c); }

    if (commandStr == "l")
    {
        HandleNewLine();
    }
    else if (commandStr == "r")
    {
        HandleLineDelete();
    }
    else if (commandStr == "build" || commandStr == "b")
    {
        assert(false && "Not implemented");
    }
    else if (commandStr == "ls")
    {
        ListCurrentDirectory();
    }
    else if (commandStr == "cd")
    {
        ChangeDirectory(argv.at(1));
    }
    else
    {
        WrongUsage(Command::None);
    }

    for (int i = 0; i < argc; i += 1) argv.at(i) = "null";
    std::cout << fs::current_path().string() << "> ";
}

void 
CommandHandler::HandleNewLine()
{
    std::string lineNumberStr = "0";

    const auto& parent = GetDirInfo();
    std::cout << dirInfo.numberOfDirs << std::endl;
    
    int lineNumber = 0;
    if (argc == 2)
    {
        if (!fs::is_empty(fs::current_path()))
        {
            /* Get the lineNumber of the last create one that is one greater and

            */
            const auto& lastDir = Dir::DirectoryInLine(parent.numberOfDirs);
            lineNumber = lastDir.lineNumber + 1;

            lineNumberStr = std::to_string(lineNumber) + " ";
            std::cout << "lineNumber: " << lineNumber << '\n';
        }
    }
    else
    {

    }

    Dir::CreateDirectory(argv.at(1).insert(0, lineNumberStr));
}

void 
CommandHandler::HandleLineDelete()
{
    int lineNum = argv[2].at(0) - '0';
    (void) lineNum;
    for (const auto& dir : Dir::PathIterator())
    {
        if (dir.path().filename().string().at(0) == argv.at(2).at(0))
        {
            if (fs::is_empty(dir.path())) 
            {
                fs::remove(dir.path());
            }
            else
            {
                std::cerr << "Not empty. And I haven't implemented deleting non-empty folders";
                exit(EXIT_FAILURE);
            }
        }
    }
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
CommandHandler::ChangeDirectory(fs::path path)
{
    fs::current_path(path);
}

