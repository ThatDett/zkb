#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <array>
#include <cstdint>
#include <string>
#include <filesystem>

class CommandHandler
{
public:

    CommandHandler();

    void Handle();

    static constexpr int MAX_ARGS = 5;
    using ArgvT = std::array<std::string, MAX_ARGS>;
    struct ArgT
    {
        ArgvT    v;
        uint32_t c;
    } arg;

    enum class Command
    {
        Line,
        Remove,
        None
    };

    static void WrongUsage(Command, bool crash = false);

private:
    void HandleNewLine();
    void HandleLineDelete(bool forceDelete = false);
    void HandleLineChange();
    void HandleUndo();
    void HandleRedo();

    void ShowStatus();
    void ListCurrentDirectory();

    void ChangeDirectory(const std::filesystem::path&);
    void ChangeDirectory(const uint64_t& lineNumber);

    void DebugRefresh();
private:
    std::filesystem::path basedPath;
};

#endif


