#ifndef COMMAND_HANDLE_HPP
#define COMMAND_HANDLE_HPP
#include <string>
#include <vector>
#include <filesystem>

class CommandHandler
{
public:
    CommandHandler(char** argv, int argc);

private: 
    enum class Command
    {
        Line,
        None
    };

    void HandleNewLine();
    void HandleLineDelete();

    void WrongUsage(Command);
    void UpdateNumberOfDirs();

    std::vector<std::string> argv;
    int                      argc;

    std::filesystem::directory_iterator dirIt{std::filesystem::current_path()};
    uint64_t lineNumber   = 0;
    uint64_t numberOfDirs = 0;
};

#endif


