#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <functional>
#include <filesystem>
#include <iterator>
#include <ostream>
#include <string>
#include <string_view>
#include <array>
#include <vector>

#include "other/CMakeVariables.h"
#include "CommandHandler.hpp"
#include "Directory.hpp"
#include "Utils.hpp"
#include "Helper.hpp"

namespace fs = std::filesystem;
using    Dir = zkb::Directory;

namespace zkb
{
    class Error 
    {
    public:
        static bool NoRootDirectory(const fs::path& path)
        {
            return path.string().size() <= 3;
        }

        static bool NonExistantLine(uint64_t lineNumber)
        {
            if (lineNumber <= 0 || lineNumber > Dir::GetNumberOfDirs())
            {
                std::cerr << "Line " << lineNumber << " is non-existant\n";
                return true;
            }
            return false;
        }

        static bool NonExistantLine(uint64_t lowerBound, uint64_t upperBound)
        {
            if (lowerBound <= 0 || upperBound > Dir::GetNumberOfDirs())
            {
                std::cerr << "Range (" << lowerBound << ',' << upperBound << ") is non-existant\n";
                return true;
            }
            return false;
        }

    private:
        Error();
    };
}

void 
CommandHandler::WrongUsage(Command command, bool crash /* = false*/)
{
    switch (command)
    {
        case Command::Line:
        {
            std::cerr << "Error adding a line: Wrong usage\n";
        } break;
        case Command::Delete:
        {
            std::cerr << "Error removing line(s): Wrong usage\n";
        } break;
        case Command::Change:
        {
            std::cerr << "Error changing line(s): Wrong usage\n";
        } break;
        case Command::Move:
        {
            std::cerr << "Error moving line(s): Wrong usage\n";
        } break;
        case Command::LS:
        {
            std::cerr << "Error listing line(s): Wrong usage\n";
        } break;
        default:
        {
            std::cerr << "Wrong Usage\n";
        } break;
    }
    if (crash) exit(EXIT_FAILURE);
}

void 
CommandHandler::WrongUsage(Setup setupError, bool crash /* = false*/)
{
    switch (setupError)
    {
        case Setup::RootDirectory:
        {
            std::cerr << "No root directory!!!\n";
            zkb::Helper::RootDirectory();
        } break;
        default:
        {
            std::cerr << "Wrong Usage\n";
        } break;
    }
    if (crash) exit(EXIT_FAILURE);
}

fs::path CommandHandler::basedPath = fs::current_path();
fs::path CommandHandler::rootPath  = CommandHandler::basedPath;

CommandHandler::CommandHandler()
{
    while (true)
    {
        if (zkb::Error::NoRootDirectory(rootPath))
        {
            WrongUsage(Setup::RootDirectory);
            return;
        }

        const auto& rootStr  = rootPath.filename().string();

        auto startPos = rootStr.find('.');
        if (startPos != std::string::npos)
        {
            if (rootStr.substr(startPos, 4) == ".zkb")
            {
                break;
            }
        }
        rootPath = rootPath.parent_path();
    }

    std::string command;
    bool quit = false;
    
    std::cout << fs::current_path().string() << "> ";
    while (!quit && std::getline(std::cin, command))
    {
        command += ' ';
        size_t pos     = command.find(' ');
        size_t wordBeg = 0;

        //Splitting
        arg.c = 0;
        for (; pos != std::string::npos; arg.c += 1)
        {
            if (arg.c >= CommandHandler::MAX_ARGS)
            {
                std::cerr << "Too many args. argc = " << arg.c << "\n";
                exit(EXIT_FAILURE);
            }
            
            const auto& subStr = command.substr(wordBeg, pos - wordBeg);
            arg.v.at(arg.c) = std::move(subStr);
            wordBeg = pos + 1;

            auto nextPos = command.find(' ', wordBeg);
            pos = nextPos;
        }

        quit = Handle();
    }
}

bool
CommandHandler::Handle()
{
    auto& commandStr = arg.v.at(0);

    uint32_t repetionNumber = 1; 
    if (zkb::IsInteger(commandStr))
    {
        //If not, arg.v.at(0) is not actually the commandStr, so we need to shift everything
        repetionNumber = std::stoll(commandStr);
        uint32_t originalArgC = arg.c;
        for (uint32_t i = 0; i < originalArgC - 1; i += 1)
        {
            arg.v.at(i) = std::move(arg.v.at(i + 1));
        }
        arg.c -= 1;
    }

    zkb::ToLower(commandStr);

    forceCommand = commandStr.at(0) == '-';
    if (forceCommand)
        commandStr    = commandStr.substr(1, commandStr.size());

    auto checkCommand = [commandStr](const std::initializer_list<const std::string_view>& args)
    {
        for (const auto& arg : args)
        {
            if (commandStr == arg)
            {
                return true;
            }
            // std::cerr << '"' << commandStr << "\" fails on arg " << arg << '\n';
        }
        return false;
    };

    if (checkCommand({"q", "quit", "exit"})) return true;

    auto evaluate = [&]() -> void(CommandHandler::*)()
    {
        // std::cerr << "Command: ";
        // for (const auto& arg : arg.v)
        // {
        //     std::cerr << arg << ", ";
        // }
        // std::cerr << "\nargc = " << arg.c << '\n';

        using CH = CommandHandler;
        if (checkCommand({"l", "line"}))
        {
            return &CH::HandleNewLine;
        }
        else if (checkCommand({"d", "delete"}))
        {
            return &CH::HandleLineDelete;
        }
        else if (checkCommand({"c", "change"}))
        {
            return &CH::HandleLineChange;
        }
        else if (checkCommand({"u", "undo"}))
        {
            return &CH::HandleUndo;
        }
        else if (checkCommand({"r", "redo"}))
        {
            return &CH::HandleRedo;
        }
        else if (checkCommand({"s", "status"}))
        {
            return &CH::ShowStatus;
        }
        else
        {
            return nullptr;
        }
    };

    auto func = evaluate();
    if (func == nullptr)
    {
        repetionNumber = 0;
    }

    for (uint32_t i = 0; i < repetionNumber; i += 1)
    {
        std::invoke(func, this);
    }

    if (repetionNumber == 0)
    {
        //Non repeatable commands
        if (checkCommand({"cd"}))
        {
            if (zkb::IsInteger(arg.v.at(1)))
                ChangeDirectory(std::stoll(arg.v.at(1)));
            else
                ChangeDirectory(arg.v.at(1));
        }
        else if (commandStr == "ref")
        {
            DebugRefresh();
        }
        else if (commandStr == "clean")
        {
            Dir::RecursivelyDelete(fs::directory_entry(fs::current_path()), false);
        }
        else if (checkCommand({"info"}))
        {
            GetDirInfo();
        }
        else if (checkCommand({"ls"}))
        {
            ListCurrentDirectory();
        }
#if DEBUG_BUILD
        else if (checkCommand({"b"}))
        {
            Command command = static_cast<Command>(std::stoi(arg.v.at(1)));
            Benchmark(command, {arg.v.at(2), arg.v.at(3), arg.v.at(4), arg.v.at(5)});
        }
#endif
        else 
        {
            WrongUsage(Command::None);
        }
    }

    lastCommand = Command::None;
    std::cout << fs::current_path().string() << "> ";
    return false;
}

void 
CommandHandler::HandleNewLine()
{
    std::string& textArg       = arg.v.at(1);

    if (lastCommand != Command::Line)
    {
        lineNumberPtr = &arg.v.at(2);
        if (!ParseStringText(textArg))
        {
            finalText = std::move(textArg);
        }
    }

    const std::string& lineNumberArg = *lineNumberPtr;
    lastCommand = Command::Line;

    switch (arg.c)
    {
        case 1:
        {
            const auto& text = std::to_string(Dir::GetNumberOfDirs() + 1) + " '...'";
            Dir::CreateDirectory(text);
        } break;
        case 2:
        {
            Dir::CreateDirectory(std::to_string(Dir::GetNumberOfDirs() + 1) + " " + finalText);
        } break;
        case 3:
        {
            if (!zkb::IsInteger(lineNumberArg))
            {
                std::cerr << "Not passing a positive integer for line number\n";
                exit(EXIT_FAILURE);
            }

            const uint64_t lineNum = std::stoll(lineNumberArg);

            if (lineNum > Dir::GetNumberOfDirs() + 1 or lineNum == 0)
            {
                std::cout << "Line " << lineNumberArg << " must already exist and be greater than 0.\n";
                char response;

                std::cout << "Create at last line? y/n: " << std::flush;
                std::cin  >> response;

                if (std::tolower(response) == 'y')
                {
                    Dir::CreateDirectory(finalText.insert(0, std::to_string(Dir::GetNumberOfDirs() + 1) + " "));
                }
                return;
            }

            for (auto& elem : fs::directory_iterator(basedPath))
            {
                auto dirLineNumber = Dir::GetDirectoryLineNumber(elem.path());

                if (dirLineNumber >= lineNum)
                {
                    Dir::ChangeDirectoryLineNumber(elem, dirLineNumber + 1);
                }
            }
            
            Dir::CreateDirectory(lineNumberArg + " " + finalText, basedPath);
        } break;
        default: WrongUsage(Command::Line); break;
    }

    // std::cout << "Parent: " << basedPath.parent_path().string() << '\n';
    // std::cout << "Create " << finalText << " at " << lineNumber << "\n";
}

void
CommandHandler::HandleLineDelete()
{
    zkb::String lineNumberArg = arg.v.at(1);
    const auto& numberOfDirs  = Dir::GetNumberOfDirs();

    if (numberOfDirs == 0)
    {
        std::cout << "Already empty\n";
        return;
    }

    isRanged = ParseRange(); 
    if (isRanged)
    {
        if (range.text.at(0) == "-1") return;
    }

    const uint64_t& lowerBound = range.num.at(0);
    const uint64_t& upperBound = range.num.at(1);

    if (lowerBound > upperBound)
    {
        std::cerr << "Lower bound (" << lowerBound << ") cannot be > upperbound (" << upperBound << ").\n";
        return;
    }

    std::vector<fs::directory_entry> dirs;

    dirs.reserve(upperBound - lowerBound + isRanged);
    if (isRanged)
    {
        RangedDirectoryIteration([&](const fs::directory_entry& elem, uint64_t)
        {
            dirs.emplace_back(std::move(elem));
        });
    }
    else
    {
        dirs.emplace_back();
    }

    auto& dir = dirs.at(0);

    lastCommand = Command::Delete;
    switch (arg.c)
    {
        case 1:
        {
            dir = Dir::DirectoryInLine(numberOfDirs); 
        } break;
        case 2:
        {
            if (isRanged) break;
            if (!zkb::IsInteger(lineNumberArg))
            {
                std::cerr << "Not passing an integer for line number\n";
                return;
            }

            dir = Dir::DirectoryInLine(std::stoll(lineNumberArg));
            std::cerr << "Dir in line is " << dir.path().filename().string() << '\n';
        } break;
        default: WrongUsage(Command::Delete); break;
    }

    for (const auto& elem : Dir::PathIterator())
    {
        if (!fs::is_empty(elem) and !forceCommand)
        {
            std::cerr << "Trying to delete a non-empty directory."
                "To confirm command use [-d|-delete].\n";
            return;
        }
    }

    for (uint64_t i = 0; i < upperBound - lowerBound + isRanged; i += 1)
    {
        if (fs::is_empty(dirs.at(i))) 
        {
            Dir::RemoveDirectory(dirs.at(i));
            continue;
        }

        Dir::RecursivelyDelete(dirs.at(i));
    }

    if (upperBound == numberOfDirs) return;
    for (auto& elem : Dir::PathIterator())
    {
        auto lineNumber = Dir::GetDirectoryLineNumber(elem);
        if (lineNumber > (isRanged? upperBound : std::stoll(lineNumberArg)))
        {
            Dir::ChangeDirectoryLineNumber(elem, lineNumber - (upperBound - lowerBound + isRanged));
        }
    }
}

void
CommandHandler::HandleLineChange()
{
    //Raw text or line number of a another directory
    std::string& arg1          = arg.v.at(1);

    if (!ParseStringText(arg1))
    {
        finalText = arg1;
    }
    else 
    {
        if (range.text.at(0) == "-1")
            return;
    }

    lastCommand = Command::Change;

    const std::string& lineNumberArg = *lineNumberPtr;
    switch (arg.c)
    {
        case 2:
        {
            uint64_t    lineNumber          = Dir::GetNumberOfDirs();
            std::string lineNumberStr       = std::to_string(lineNumber);

            const auto& dir                 = Dir::DirectoryInLine(lineNumber);
            const auto& parent              = dir.path().parent_path().string();

            const auto& saveName = Dir::GetDirectoryName(dir).insert(0, "\"") + '"';
            const auto& rangeStr = std::string("(") + lineNumberStr + "," + lineNumberStr + ")";

            //Save
            Dir::history.push({{{"c", saveName, rangeStr}, 3}, basedPath});
            
            fs::rename(dir.path(), parent + '\\' + lineNumberStr + ' ' + finalText);

        } break;
        case 3:
        {
            isRanged = ParseRange();
            if (isRanged)
            {
                if (range.text.at(0) == "-1") return;
            }

            auto& lowerBound = range.num.at(0);
            auto& upperBound = range.num.at(1);

            if (!isRanged)
            {
                lowerBound = std::stoll(lineNumberArg);
                upperBound = lowerBound;
            }

            if constexpr (DEBUG_BUILD)
            {
                std::cout << "lowerBound: " << lowerBound << '\n';
                std::cout << "upperBound: " << upperBound << '\n';
            }

            if (zkb::Error::NonExistantLine(lowerBound, upperBound)) return;
            if (zkb::IsInteger(arg1))
            {
                RangedDirectoryIteration([arg1](const fs::directory_entry& elem, uint64_t lineNumber)
                {
                    const auto& parent = elem.path().parent_path().string();
                    fs::rename(elem.path(), parent + "\\" + std::to_string(lineNumber) + ' ' + Dir::GetDirectoryName(Dir::DirectoryInLine(std::stoll(arg1))));
                });
            }
            else
            {
                RangedDirectoryIteration([arg1, this](const fs::directory_entry& elem, uint64_t lineNumber)
                {
                    const auto& parent = elem.path().parent_path().string();
                    fs::rename(elem.path(), parent + "\\" + (std::to_string(lineNumber) + ' ') + finalText);
                });
            }
        } break;
        default:
        {
            WrongUsage(Command::Change);
            return;
        }
    }
}

void
CommandHandler::HandleLineMove()
{
    
}

void
CommandHandler::HandleUndo()
{
    auto& history = Dir::history;
    if (history.empty())
    {
        std::cerr << "No changes to undo\n";
        return;
    }

    bool repeat = arg.v.at(1) == "all";
    // repeat = true;

    // while (!history.empty())
    // {
        Dir::HistoryT& historyV = history.top();
        
        arg.v = historyV.args.v;
        arg.c = historyV.args.c;

        basedPath = std::move(historyV.path);
    
        Handle();
        history.pop();
        // if (!repeat) break;
    // }
    // basedPath = fs::current_path();
}

void
CommandHandler::HandleRedo()
{
    std::cerr << "Not implemented. Forwading to undo command\n";
    HandleUndo();
}

void
CommandHandler::ShowStatus()
{
    std::cout << "Number of lines: " << Dir::GetNumberOfDirs() << '\n';
}

void
CommandHandler::GetDirInfo()
{
    if (arg.c != 2)
    {
        WrongUsage(Command::None);
        return;
    }

    const auto& dir = Dir::DirectoryInLine(std::stoll(arg.v.at(1)));

    std::cout << "Text: "         << Dir::GetDirectoryName(dir) << '\n';
    std::cout << "Is directory: " << fs::is_directory(dir)      << '\n';
    std::cout << "Empty: "        << fs::is_empty(dir)          << std::endl;
}

void 
CommandHandler::ListCurrentDirectory()
{
    // const auto start{std::chrono::steady_clock::now()};
    std::cout << '\n';
    if (ParseStringText(arg.v.at(1)))
    {
        if (range.text.at(0) == "-1") return;
    }

    switch (arg.c)
    {
        case 1:
        {
            uint64_t lineNumber = 1;
            if (!fs::is_empty(basedPath))
            {
                for (const auto& dir : Dir::PathIterator())
                {
                    uint64_t dirLineNumber = Dir::GetDirectoryLineNumber(dir);
                    if (dirLineNumber == lineNumber or forceCommand)
                        std::cout << "\t\t\t\t" << dir.path().filename().string() << '\n';
                    else
                    {
                        for (const auto& dir : Dir::PathIterator())
                        {
                            uint64_t otherdirLineNumber = Dir::GetDirectoryLineNumber(dir);
                            if (otherdirLineNumber == lineNumber)
                                std::cout << "\t\t\t\t" << dir.path().filename().string() << '\n';
                        }
                    }
                    lineNumber += 1;
                }
                std::cout << "\t\t\t"; ShowStatus();
            }
            else
            {
                std::cout << "Number of lines: 0" << '\n';
            }
        }
        case 2:
        {
            if (zkb::IsInteger(arg.v.at(1)))
            {
                const auto& dir = Dir::DirectoryInLine(std::stoll(arg.v.at(1)));
                std::cout << "\t\t\t\t" << dir.path().filename().string() << "\n";
            }
            else
            {
                bool notFound = true;
                for (const auto& elem : Dir::PathIterator())
                {
                    const auto& name = Dir::GetDirectoryName(elem);
                    if (name == finalText)
                    {
                        std::cout << "\t\t\t\t" << elem.path().filename().string() << '\n';
                        notFound = false;
                    }
                }

                if (notFound)
                {
                    std::cout << "\t\t\t\tNo match\n";
                }
            }

        } break;
        default:
        {
            std::cerr << arg.c << '\n';
            WrongUsage(Command::LS);
        }
    }
    std::cout << std::endl;
    // const auto end{std::chrono::steady_clock::now()};
    // const std::chrono::duration<double> elapsed_seconds{end - start};
    // std::cout << "Took:" << elapsed_seconds.count() << " with forceCommand: " << forceCommand << std::endl;
}

void
CommandHandler::ChangeDirectory(const fs::path& path)
{
    if (path.string().at(1) != ':' && !path.string().starts_with('.'))
    {
        std::cerr << "Path doesn't exist\n";
        return;
    }

    fs::current_path(path);
}

void
CommandHandler::ChangeDirectory(const uint64_t& lineNumber)
{
    fs::current_path(Dir::DirectoryInLine(lineNumber).path());
}

void CommandHandler::DebugRefresh()
{
    // uint64_t lineOfNonexistantDir = 0;
    // for (uint64_t line = 1; line < Dir::GetNumberOfDirs(); line += 1)
    // {
    //     std::cerr << "line = " << line << "\nlineOfNon = " << lineOfNonexistantDir << '\n';
    //     const auto dir = Dir::DirectoryInLine(line);
    //     if (lineOfNonexistantDir != 0)
    //     {
    //         if (line <= lineOfNonexistantDir) continue;
    //         for (auto& elem : Dir::PathIterator())
    //         {
    //             uint64_t lineNumber = Dir::GetDirectoryLineNumber(elem);
    //             if (lineNumber <= line - lineOfNonexistantDir) continue;
    //
    //             std::cerr << "original: " << lineNumber << "\nnew: " << lineNumber - (line - lineOfNonexistantDir + 1);
    //             Dir::ChangeDirectoryLineNumber(elem, lineNumber - (line - lineOfNonexistantDir + 1));
    //         }
    //     }
    //     if (dir.path().filename().string() == "null") lineOfNonexistantDir = line;
    // }
}

bool
CommandHandler::ParseStringText(std::string& textArg)
{
    static constexpr char stringChar = '"';
    if (zkb::IsInteger(textArg) || !textArg.starts_with('"'))
    {
        std::cerr << "Can't parse string\n";
        return false;
    }

    if (textArg.size() < 3)
    {
        WrongUsage(Command::None);
        std::cerr << "Malformed string\n";
        range.text.at(0) = "-1";
        return true;
    }

    finalText = "";
    textArg = textArg.substr(1, textArg.size() - 1);
    auto beg = std::next(std::begin(arg.v));

    uint32_t originalArgc = arg.c;
    for (uint32_t i = 0; i < originalArgc - 1; ++beg, i += 1)
    {
        const auto& elem = *beg;
        if (i == originalArgc - 2 or (elem.ends_with(stringChar) and elem.at(elem.size() - 2) != '\\'))
        {
            // std::cerr << elem.substr(0, elem.size() - elem.ends_with(stringChar)) << '\n';
            finalText += elem.substr(0, elem.size() - elem.ends_with(stringChar));
            break;
        }

        arg.c -= 1;
        finalText += elem + ' ';
    }
    
    if (arg.c == 3)
    {
        lineNumberPtr = &arg.v.at(originalArgc - 1);
    }
    // std::cerr << finalText << ";\n";
    return true;
}

bool
CommandHandler::ParseRange()
{
    const auto& string = *lineNumberPtr;
    if (!string.starts_with('(') or !string.ends_with(')'))
    {
        return false;
    }

    uint64_t pos[2] = {0};
    for (const char& ch : string)
    {
        if (std::isdigit(ch)) break;
        if (ch == ',') 
        {
            pos[0] = 0;
            break;
        }

        pos[0] += 1;
    }

    pos[1] = string.find(',');
    if (pos[1] == std::string::npos)
    {
        WrongUsage(Command::None);
        std::cerr << "Malformed range";
        range.text.at(0) = "-1";
        return true;
    }

    if (!forceCommand)
    {
        WrongUsage(Command::Change);
        std::cerr << "Use -c to change a range of lines\n";
        range.text.at(0) = "-1";
        return true;
    }
    
    range.text.at(0) = pos[0] > 0? string.substr(pos[0], pos[1] - 1) : "1";
    const auto& upperBound = string.substr(pos[1] + 1, string.size() - 1);

    for (const char& ch : upperBound)
    {
        if (std::isdigit(ch)) break;
        if (ch == ')') 
        {
            pos[1] = 0;
        }
    }

    range.text.at(1) = pos[1] > 0? string.substr(pos[1] + 1, string.size() - 1) : std::to_string(Dir::GetNumberOfDirs());

    for (const auto& str : range.text)
    {
        std::cerr << "range: " << str << '\n';
    }

    for (int i = 0; i < 2; i += 1)
    {
        std::cerr << "inside\n";
        range.num.at(i) = std::stoll(range.text.at(i));
        if (zkb::Error::NonExistantLine(range.num.at(i)))
        {
            std::cerr << "Error at ranged delete\n";
            range.text.at(0) = "-1";
            return false;
        }
    }
    return true;
}

void
CommandHandler::RangedDirectoryIteration(std::function<void(const std::filesystem::directory_entry&, uint64_t)> func)
{
    const auto& lowerBound = range.num.at(0);
    const auto& upperBound = range.num.at(1);

    bool equal = lowerBound == upperBound;

    for (const auto& elem : fs::directory_iterator{basedPath})
    {
        if (!fs::is_directory(elem)) continue;

        uint64_t dirLineNumber = Dir::GetDirectoryLineNumber(elem);
        if (dirLineNumber >= lowerBound && dirLineNumber <= upperBound)
        {
            func(elem, dirLineNumber);
        }
    }
}

#if DEBUG_BUILD
void 
CommandHandler::Benchmark(Command command, const std::array<std::string, 4>& arr)
{
    if (arg.v.at(1) == "commands")
    {
        std::cout << '\n' <<
            "Line = "   << (int)Command::Line   << '\n' <<
            "Delete = " << (int)Command::Delete << '\n' <<
            "Move = "   << (int)Command::Move   << '\n' <<
            "List = "   << (int)Command::LS     << '\n' <<
            "CD   = "   << (int)Command::CD     << "\n\n"
            ;
    }


    const auto& timesToRepeat = arr.at(0);
    const auto& numberOfLines = arr.at(1);

    switch (command)
    {
        case Command::Line:
        {
            std::vector<double> markings;
            markings.reserve(std::stoi(timesToRepeat));
            for (uint32_t i = 0; i < std::stoi(timesToRepeat); i += 1)
            {
                arg.c = 3;
                arg.v.at(0) = numberOfLines;
                arg.v.at(1) = "l";
                arg.v.at(2) = "test";

                const auto start{std::chrono::steady_clock::now()};
                Handle();
                const auto end{std::chrono::steady_clock::now()};
                const std::chrono::duration<double> elapsed_seconds{end - start};
                markings.push_back(elapsed_seconds.count());

                arg.c = 2;
                arg.v.at(0) = "-d";
                arg.v.at(1) = "(,)";
                HandleLineDelete();
            }
        } break;
        case Command::LS:
        {
            arg.c = 1;
            arg.v.at(0) = "ls";

            std::vector<double> markings;
            std::vector<double> secondMarkings;
            markings.reserve(std::stoi(timesToRepeat));
            for (uint32_t i = 0; i < 2; i += 1)
            {
                for (uint32_t j = 0; j < std::stoi(timesToRepeat); j += 1)
                {
                    const auto start{std::chrono::steady_clock::now()};
                    Handle();
                    const auto end{std::chrono::steady_clock::now()};
                    const std::chrono::duration<double> elapsed_seconds{end - start};
                    if (i == 0)
                        markings.push_back(elapsed_seconds.count());
                    else
                        secondMarkings.push_back(elapsed_seconds.count());
                }
                arg.v.at(0) = "-ls";
            };

            double average = 0;
            for (const auto& elem : markings)
            {
                average += elem;
            }
            average /= std::stoi(timesToRepeat);

            double secondAverage = 0;
            for (const auto& elem : secondMarkings)
            {
                secondAverage += elem;
            }
            secondAverage /= std::stoi(timesToRepeat);

            std::cerr << "Average time took ordered: " << average << '\n';
            std::cerr << "Average time took unordered: " << secondAverage << '\n';
        } break;
        default:
        {
        } break;
    }
}
#endif
