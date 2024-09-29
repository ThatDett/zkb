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
        Remove,
        Change,
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
    void HandleUndo();
    void HandleRedo();

    void ShowStatus();
    void GetDirInfo();
    void ListCurrentDirectory();

    void ChangeDirectory(const std::filesystem::path&);
    void ChangeDirectory(const uint64_t& lineNumber);

    bool ParseStringText(std::string& textArg, std::string*& lineNumberArg);
    bool ParseRange(const std::string&, RangeT&);
    void RangedDirectoryIteration(std::function<void(const std::filesystem::directory_entry&, uint64_t)>);

    void DebugRefresh();

#if DEBUG_BUILD
    void Benchmark(Command, const std::array<std::string, 4>&);
#endif
private:
    RangeT range;
    bool   isRangedDelete;
    bool   forceCommand;
};

#endif


