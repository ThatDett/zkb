#include <cctype>
#include <cstdlib>
#include <iostream>
#include <filesystem>

#include "CommandHandle.hpp"

enum class Command
{
    Line,
    None
};

void WrongUsage(Command command)
{
    std::cout << "Wrong usage\n";
    exit(EXIT_FAILURE);
}

void CommandHandle(char** argv, int argc)
{
    namespace fs = std::filesystem;

    const auto& exePath = fs::current_path();

    Command command = Command::None;
    if (argc == 1) { WrongUsage(command); }

    fs::directory_iterator dirIt(exePath);
    uint64_t lineNumber{};
    uint64_t numberOfDirs{};

    std::string commandStr(argv[1]);
    for (char& c : commandStr) { c = std::tolower(c); }
    if (commandStr == "l")
    {
        for (const auto& throwAway : fs::directory_iterator(exePath))
        {
            numberOfDirs += 1;            
        }
        
        char lineNumberChar[2];
        {size_t i = 0;
        for (const auto& dir : dirIt)
        {
            std::cout << dir << '\n';
            
            if (i == numberOfDirs - 1)
            {
                lineNumberChar[0] = dir.path().filename().string().at(0);
                lineNumber = lineNumberChar[0] - '0';
                std::cout << "lineNumber: " << lineNumber << '\n';
            }
            i += 1; 
        }}
        lineNumberChar[1] = '\0';
        //std::cout << std::string(lineNumberChar) + " " + argv[2] + "\n";
        lineNumberChar[0] += 1;
        std::filesystem::create_directory(std::string(lineNumberChar) + " " + argv[2]);
    }
}
