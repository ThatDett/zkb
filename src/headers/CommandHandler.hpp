#ifndef COMMAND_HANDLE_HPP
#define COMMAND_HANDLE_HPP
#include <array>
#include <string>
#include <filesystem>
#include <unordered_map>

#include "Directory.hpp"

class CommandHandler
{
public:
    static constexpr int MAX_ARGS = 5;
    using ArgvT = std::array<std::string, MAX_ARGS>;

   CommandHandler(ArgvT&);

    void Handle(int argc);
    static auto GetDirInfo(const std::string& = zkb::fs::current_path().string()) -> zkb::Directory&;

private:
    ArgvT& argv;
    int    argc;

    enum class Command
    {
        Line,
        None
    };

    void WrongUsage(Command, bool crash = false);

    void HandleNewLine();
    void HandleLineDelete();

    void ListCurrentDirectory();
    void ChangeDirectory(zkb::fs::path);
};

#endif


