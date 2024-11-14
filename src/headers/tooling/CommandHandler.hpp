#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <filesystem>

#include "other/CMakeVariables.h"

namespace zkb 
{
    class Directory;
}

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

    struct HistoryT
    {
        ArgT   args;
        std::filesystem::path  path;
    };

    struct RangeT
    {
        std::array<std::string, 2> text;
        std::array<uint32_t,    2> num{0, 1};
    };

public:
    enum class Command
    {
        Line,
        SetLine,
        Delete,
        Change,
        Move,
        Swap,
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

private:
    void ShowBasedPath();
public:
    static std::filesystem::path basedPath;
    static std::filesystem::path rootPath;

private:

    void HandleNewLine();
    void SetCurrentLine();
    void HandleLineDelete();
    void HandleLineChange();
    void HandleLineSwap();
    void HandleUndo();
    void HandleRedo();

    void ShowStatus();
    void GetDirInfo();
    void ListCurrentDirectory();

    void ChangeDirectory();

    bool ParseStringText(std::string& textArg);
    bool ParseRange(std::string&, Command);

    //For changing directories
    bool ParsePath(const std::string&);

    void RangedDirectoryIteration(std::function<void(const std::filesystem::directory_entry&, uint32_t)>);
    void GenericDirectoryIteration(std::function<void(zkb::Directory)>);

    void DebugRefresh();

#if DEBUG_BUILD
    void Benchmark(Command, const std::array<std::string, 4>&);

#endif
private:
    RangeT  range;
    uint32_t currentLine    = 1;
    uint32_t repetionNumber;
    uint32_t iteration;

    //Final text of the string
    std::string        finalText;
    //Points to the actual line number in string args
    std::string* lineNumberPtr = nullptr;

    bool               isRanged;
    bool               forceCommand;
    Command            lastCommand = Command::None;

    bool updateDirectoriesList = true;
};

#endif


