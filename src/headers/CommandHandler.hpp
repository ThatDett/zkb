#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <array>
#include <cstdint>
#include <string>
#include <filesystem>

#include "Directory.hpp"

class CommandHandler
{
public:
    static constexpr int MAX_ARGS = 5;
    using ArgvT = std::array<std::string, MAX_ARGS>;

    CommandHandler();

    void Handle();

private:
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

    void WrongUsage(Command, bool crash = false) const;

    void HandleNewLine();
    void HandleLineDelete();

    void ListCurrentDirectory();
    void ChangeDirectory(const zkb::fs::path&);
};

#endif


