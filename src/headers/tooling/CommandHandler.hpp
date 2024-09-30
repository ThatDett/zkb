#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <filesystem>

#include "other/CMakeVariables.h"

class CommandHandler
{
public:
    CommandHandler();

public:
    static constexpr int MAX_ARGS = 24;
    using ArgvT = std::array<std::string, MAX_ARGS>;
    struct ArgT
    {
        ArgvT    v;
        uint32_t c;
    } arg;

    struct RangeT
    {
        std::array<std::string, 2> text;
        std::array<uint32_t,    2> num{0, 1};
    };

    enum class Command
    {
        Line,
        Delete,
        Change,
        Move,
        Undo,
        Redo,
        LS,
        CD,
        None
    };

    enum class Setup
    {
        RootDirectory,
    };

public:
    bool Handle();

    static void WrongUsage(Command, bool crash = false);
    static void WrongUsage(Setup, bool   crash = false);

public:
    static std::filesystem::path basedPath;
    static std::filesystem::path rootPath;

private:

    void HandleNewLine();
    void HandleLineDelete();
    void HandleLineChange();
    void HandleLineMove();
    void HandleUndo();
    void HandleRedo();

    void ShowStatus();
    void GetDirInfo();
    void ListCurrentDirectory();

    void ChangeDirectory(const std::filesystem::path&);
    void ChangeDirectory(const uint64_t& lineNumber);

    bool ParseStringText(std::string& textArg);
    bool ParseRange();
    void RangedDirectoryIteration(std::function<void(const std::filesystem::directory_entry&, uint64_t)>);

    void DebugRefresh();

#if DEBUG_BUILD
    void Benchmark(Command, const std::array<std::string, 4>&);

#endif
private:
    bool benchmark;

    RangeT  range;

    //Final text of the string
    std::string        finalText;

    //Points to the actual line number in string args
    const std::string* lineNumberPtr;
    bool               isRanged;
    bool               forceCommand;
    Command            lastCommand = Command::None;
};

#endif


