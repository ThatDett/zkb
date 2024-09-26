#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

#include "CommandHandler.hpp"
#include "Directory.hpp"
#include "Utils.hpp"

namespace fs = std::filesystem;
using    Dir = zkb::Directory;

namespace zkb
{
    class Error 
    {
    public:
        static bool NonExistantLine(uint64_t lineNumber)
        {
            if (lineNumber <= 0 || lineNumber > Dir::GetNumberOfDirs())
            {
                std::cerr << "Line " << lineNumber << " is non-existant\n";
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
        case Command::Remove:
        {
            std::cerr << "Error removing line(s): Wrong usage\n";
        } break;
        default:
        {
            std::cerr << "Wrong Usage\n";
        } break;
    }
    if (crash) exit(EXIT_FAILURE);
}

CommandHandler::CommandHandler()
{
    std::string command;
    
    std::cout << fs::current_path().string() << "> ";
    while (std::getline(std::cin, command))
    {
        command += ' ';
        size_t pos     = command.find(' ');
        size_t wordBeg = 0;

        //Splitting
        arg.c = 0;
        for (; pos != std::string::npos; arg.c += 1)
        {
            if (arg.c > CommandHandler::MAX_ARGS)
            {
                std::cerr << "Too many args. argc = " << arg.c << "\n";
                exit(EXIT_FAILURE);
            }
            
            const auto& subStr = command.substr(wordBeg, pos - wordBeg);
            arg.v.at(arg.c) = subStr;
            wordBeg = pos + 1;

            auto nextPos = command.find(' ', wordBeg);
            pos = nextPos;
        }

        Handle();
    }
}

void 
CommandHandler::Handle()
{
    auto& commandStr = arg.v.at(0);
    if (commandStr == "quit" or commandStr == "q" || commandStr == "exit") exit(EXIT_SUCCESS);

    zkb::ToLower(commandStr);

    uint64_t repetionNumber = 1; 
    if (zkb::IsInteger(commandStr))
    {
        //If not, arg.v.at(0) is not actually the commandStr, so we need to shift everything
        repetionNumber = std::stoll(commandStr);
        for (int i = 0; i < arg.c; i += 1)
        {
            arg.v.at(i) = arg.v.at(i + 1);
        }
        arg.c -= 1;
    }

    bool forceCommand = commandStr.at(0) == '-';
    if (forceCommand)
        commandStr    = commandStr.substr(1, commandStr.size());

    auto checkCommand = [commandStr](std::initializer_list<const std::string_view> args)
    {
        for (const auto& arg : args)
        {
            if (commandStr == arg) return true;
        }
        return false;
    };

    auto evaluate = [&]()
    {
        // std::cerr << "Command: ";
        // for (const auto& arg : arg.v)
        // {
        //     std::cerr << arg << ", ";
        // }
        // std::cerr << '\n';

        if (checkCommand({"l", "line"}))
        {
            HandleNewLine();
        }
        else if (checkCommand({"d", "delete"}))
        {
            HandleLineDelete(forceCommand);
        }
        else if (checkCommand({"c", "change"}))
        {
            HandleLineChange();
        }
        else if (checkCommand({"u", "undo"}))
        {
            HandleUndo();
        }
        else if (checkCommand({"r", "redo"}))
        {
            HandleRedo();
        }
        else if (checkCommand({"s", "status"}))
        {
            ShowStatus();
        }
        else if (checkCommand({"ls"}))
        {
            ListCurrentDirectory();
        }
        else if (checkCommand({"cd"}))
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
        else
        {
            WrongUsage(Command::None);
        }
    };

    for (int i = 0; i < repetionNumber; i += 1)
    {
        evaluate();
    }


    std::cout << fs::current_path().string() << "> ";
}

void 
CommandHandler::HandleNewLine()
{
    std::string  lineNumberStr;
    std::string  textArg       = arg.v.at(1);
    std::string& lineNumberArg = arg.v.at(2);

    std::cout << "Parent: " << basedPath.parent_path().string() << '\n';
    std::cout << "Create " << textArg << " at " << lineNumberArg << "\n";

    switch (arg.c)
    {
        case 1:
        {
            Dir::CreateDirectory(std::to_string(Dir::GetNumberOfDirs()));
        } break;
        case 2:
        {
            uint64_t    numberOfLines = Dir::GetNumberOfDirs();
            lineNumberStr = std::to_string(numberOfLines + 1) + " ";
            Dir::CreateDirectory(textArg.insert(0, lineNumberStr));
        } break;
        case 3:
        {
            if (!zkb::IsInteger(lineNumberArg))
            {
                std::cerr << "Not passing an integer for line number\n";
                exit(EXIT_FAILURE);
            }

            std::cerr << "basedPath: " << basedPath << '\n';
            for (auto& elem : fs::directory_iterator(basedPath))
            {
                auto lineNumber = Dir::GetDirectoryLineNumber(elem.path());

                if (lineNumber >= std::stoll(lineNumberArg))
                {
                    Dir::ChangeDirectoryLineNumber(elem, lineNumber + 1);
                }
            }
            
            Dir::CreateDirectory(textArg.insert(0, lineNumberArg += ' '), basedPath);
        } break;
        default: WrongUsage(Command::Line); break;
    }
}

void
CommandHandler::HandleLineDelete(bool forceDelete)
{
    zkb::String lineNumberArg = arg.v.at(1);
    const auto& numberOfDirs  = Dir::GetNumberOfDirs();

    if (numberOfDirs == 0)
    {
        std::cout << "Already empty\n";
        return;
    }

    bool isRangedDelete = false;
    if (lineNumberArg.size() > 0)
    {
        isRangedDelete = lineNumberArg.at(0) == '(';
    }
    else 
    {
        arg.c = 1;
    }

    struct
    {
        std::array<std::string, 2> text;
        std::array<uint32_t,    2> numbers{0, 1};
    } rangeParams{};

    //Parse range parameters, i.e (num1, num2)
    if (isRangedDelete)
    {
        if (arg.c < 2) WrongUsage(Command::Remove);
        
        rangeParams.text = zkb::ParseRange(lineNumberArg);

        for (int i = 0; i < 2; i += 1)
        {
            rangeParams.numbers.at(i) = std::stoll(rangeParams.text.at(i));
            if (zkb::Error::NonExistantLine(rangeParams.numbers.at(i)))
            {
                std::cerr << "Error at ranged delete\n";
                return;
            }
        }
    }

    const uint64_t& lowerBound = rangeParams.numbers.at(0);
    const uint64_t& upperBound = rangeParams.numbers.at(1);

    if (lowerBound > upperBound)
    {
        std::cerr << "Lower bound (" << lowerBound << ") cannot be > upperbound (" << upperBound << ").\n";
        return;
    }

    std::vector<fs::directory_entry> dirs;

    dirs.reserve(upperBound - lowerBound + isRangedDelete);
    if (isRangedDelete)
    {
        for (auto& elem : Dir::PathIterator())
        {
            uint64_t lineNumber = Dir::GetDirectoryLineNumber(elem);
            if (lineNumber >= lowerBound && lineNumber <= upperBound)
            {
                if (fs::is_directory(elem))
                    dirs.emplace_back(std::move(elem));
            }
        }
    }
    else
    {
        dirs.emplace_back();
    }

    auto& dir = dirs.at(0);

    switch (arg.c)
    {
        case 1:
        {
            dir = Dir::DirectoryInLine(numberOfDirs); 
        } break;
        case 2:
        {
            if (isRangedDelete) break;
            if (!zkb::IsInteger(lineNumberArg))
            {
                std::cerr << "Not passing an integer for line number\n";
                return;
            }

            dir = Dir::DirectoryInLine(std::stoll(lineNumberArg));
            std::cerr << "Dir in line is " << dir.path().filename().string() << '\n';
        } break;
        default: WrongUsage(Command::Remove); break;
    }

    for (const auto& elem : Dir::PathIterator())
    {
        if (!fs::is_empty(elem) && !forceDelete)
        {
            std::cerr << "Trying to delete a non-empty directory."
                "To confirm command use [-d|-delete].\n";
            return;
        }
    }

    for (uint64_t i = 0; i < upperBound - lowerBound + isRangedDelete; i += 1)
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
        if (lineNumber > (isRangedDelete? upperBound : std::stoll(lineNumberArg)))
        {
            Dir::ChangeDirectoryLineNumber(elem, lineNumber - (upperBound - lowerBound + isRangedDelete));
        }
    }
}

void
CommandHandler::HandleLineChange()
{
    //Raw text or line number of a another directory
    std::string& arg1                   = arg.v.at(1);

    // Change directory line text at line number arg2
    std::string& lineNumberArg          = arg.v.at(2);
    uint64_t     lineNumber             = std::stoll(lineNumberArg);

    if (zkb::Error::NonExistantLine(lineNumber)) return;

    const auto& dir        = Dir::DirectoryInLine(lineNumber);
    const auto& parentPath = dir.path().parent_path().string();

    if (zkb::IsInteger(arg1))
    {
        fs::rename(dir.path(), parentPath + "\\" + lineNumberArg + Dir::GetDirectoryName(Dir::DirectoryInLine(std::stoll(arg1))));
    }
    else
    {
        fs::rename(dir.path(), parentPath + "\\" + (lineNumberArg += ' ') + arg1);
    }
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
    repeat = true;

    while (!history.empty())
    {
        Dir::HistoryT& historyV = history.top();
        
        arg.v = historyV.args.v;
        arg.c = historyV.args.c;

        basedPath = historyV.path.string();
    
        HandleNewLine();
        history.pop();
        if (!repeat) break;
    }
    basedPath = fs::current_path();
}

void
CommandHandler::HandleRedo()
{
    HandleUndo();
}

void
CommandHandler::ShowStatus()
{
    std::cout << "Number of lines: " << Dir::GetNumberOfDirs() << '\n';
}

void 
CommandHandler::ListCurrentDirectory()
{
    std::cout << '\n';
    for (const auto& dir : Dir::PathIterator())
    {
        std::cout << "\t\t\t\t" << dir.path().filename().string() << '\n';
    }
    std::cout << "\t\t\t"; ShowStatus();
    std::cout << std::endl;
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
