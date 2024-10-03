#include <cassert>
#if DEBUG_BUILD
#include <chrono>
#endif
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <functional>
#include <filesystem>
#include <iterator>
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
        case Command::SetLine:
        {
            std::cerr << "Error setting current line: Wrong usage\n";
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
        case Command::Swap:
        {
            std::cerr << "Error swaping line(s): Wrong usage\n";
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

void 
CommandHandler::ShowBasedPath()
{
    fs::path relevantPath = basedPath;
    std::string relevantPathStr;

    while (relevantPath.filename() != rootPath.parent_path().filename())
    {
        std::string name = relevantPath.filename().string();
        fs::path parent = relevantPath.parent_path();

        relevantPathStr.insert(0, parent == rootPath.parent_path()? std::move(name) : name.insert(0, "/"));
        relevantPath = std::move(parent);
    }

    std::cout << currentLine << '|' << relevantPathStr << "> ";
}

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
 
    ShowBasedPath();
    while (!quit && std::getline(std::cin, command))
    {
        command += ' ';
        size_t pos     = command.find(' ');
        size_t wordBeg = 0;

        //Splitting
        for (arg.c = 0; pos != std::string::npos; arg.c += 1)
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
        
        auto saveLastCommand = lastCommand;
        lastCommand = Command::None;

        /* 
         * For some reason swap has a std::invalid_argument exception when you swap with 
         * upperbound + numberOfLinesToShift > numberOfDirs
         */
        try
        {
            quit = Handle();
        }
        catch (std::invalid_argument)
        {
            if (saveLastCommand != Command::Swap)
            {
                std::cerr << "std::invalid_argument\n";
                ShowBasedPath();
                continue;
            }

            if constexpr (DEBUG_BUILD)
            {
                std::cerr << "possibly unknown std::invalid_argument\n";
                ShowBasedPath();
                continue;
            }
        }
    }
}

bool
CommandHandler::Handle()
{
    auto& commandStr = arg.v.at(0);

    repetionNumber = 1; 
    if (zkb::IsInteger(commandStr))
    {
        //If not, arg.v.at(0) is not actually the commandStr, so we need to shift everything
        repetionNumber = std::stoi(commandStr);

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
        else if (checkCommand({"u", "undo"}))
        {
            return &CH::HandleUndo;
        }
        else if (checkCommand({"r", "redo"}))
        {
            return &CH::HandleRedo;
        }
        else
        {
            return nullptr;
        }
    };

    auto func = evaluate();
    if (func != nullptr)
    {
        for (iteration = 0; iteration < repetionNumber; iteration += 1)
        {
            std::invoke(func, this);
        }
    }
    else
    {
        //Non repeatable commands
        if (checkCommand({"cd"}))
        {
            ChangeDirectory();
        }
        else if (checkCommand({"c", "change"}))
        {
            HandleLineChange();
        }
        else if (checkCommand({"ls"}))
        {
            ListCurrentDirectory();
        }
        else if (checkCommand({"sl"}))
        {
            SetCurrentLine();
        }
        else if (checkCommand({"s", "swap"}))
        {
                HandleLineSwap();
        }
        else if (commandStr == "ref")
        {
            DebugRefresh();
        }
        else if (commandStr == "clean")
        {
            Dir::RecursivelyDelete(fs::directory_entry(fs::current_path()), false);
        }
        else if (checkCommand({"s", "status"}))
        {
            ShowStatus();
        }
        else if (checkCommand({"info"}))
        {
            GetDirInfo();
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

    ShowBasedPath();
    return false;
}

void 
CommandHandler::HandleNewLine()
{
    //TODO: Make this more efficient on repeat, don't need to update the directories by 1
    // could just update by repetion amount
    std::string& textArg       = arg.v.at(1);

    if (lastCommand != Command::Line)
    {
        if (arg.c > 1)
        {
            lineNumberPtr = &arg.v.at(2);
            if (!ParseStringText(textArg))
            {
                finalText = std::move(textArg);
            }
        }
        finalText += ";";
    }


    static uint32_t nameRepetion = 0;
    uint64_t numberOfDirs = Dir::GetNumberOfDirs();

    std::string& lineNumberArg = *lineNumberPtr;
    std::string finalName;

    auto nameRepetionLookup = [this](std::string& finalName)
    {
        return;
        // if (forceCommand) return;
        // // if (lastCommand == Command::Line)
        // // {
        // //     finalName += std::to_string(nameRepetion + iteration).insert(0, " (") + ")";
        // //     return;
        // // }
        //
        // nameRepetion = 0;
        // for (const auto& elem : Dir::PathIterator())
        // {
        //     const auto& name = Dir::GetDirectoryName(elem);
        //     if (name.substr(0, name.find(";")) == finalName)
        //     {
        //         std::cerr << name.substr(0, name.find(";")) << " repeats\n";
        //         nameRepetion += 1;
        //     }
        //     else
        //     {
        //         std::cerr << name.substr(0, name.find(";")) << " doesn't repeat\n";
        //     }
        // }
        //
        // if (nameRepetion != 0)
        //     finalName += std::to_string(nameRepetion).insert(0, " (") + ")";
    };

    switch (arg.c)
    {
        case 1:
        {
            finalName = std::to_string(currentLine) + " '...';";
            nameRepetionLookup(finalName);
            Dir::CreateDirectory(finalName);
        } break;
        case 2:
        {
            finalName = std::to_string(currentLine) + " " + finalText;
            nameRepetionLookup(finalName);
            Dir::CreateDirectory(finalName);
        } break;
        case 3:
        {
            if (!zkb::IsInteger(lineNumberArg))
            {
                std::cerr << "Not passing a positive integer for line number\n";
                exit(EXIT_FAILURE);
            }

            const uint64_t lineNum = std::stoll(lineNumberArg);
            if (lineNum > numberOfDirs + 1 or lineNum == 0)
            {
                std::cout << "Line " << lineNumberArg << " must already exist and be greater than 0.\n";
                char response;

                std::cout << "Create at last line? y/n: " << std::flush;
                std::cin  >> response;

                if (std::tolower(response) == 'y')
                {
                    Dir::CreateDirectory(std::to_string(numberOfDirs) + " " + finalText);
                }
                return;
            }
            
            currentLine = lineNum;

            if (iteration + 1 == repetionNumber)
            {
                lineNumberArg += " ";
                lineNumberArg += std::move(finalText);
                finalName     =  std::move(lineNumberArg);
            }
            else
                finalName = lineNumberArg + " " + finalText;

            nameRepetionLookup(finalName);
                
            Dir::CreateDirectory(finalName, basedPath);
        } break;
        default: WrongUsage(Command::Line); return;
    }

    if (lastCommand != Command::Line)
    {
        if (currentLine <= numberOfDirs)
        {
            GenericDirectoryIteration([this, finalName](zkb::DirEntry elem, uint64_t dirLineNumber)
            {
                if (dirLineNumber >= currentLine && finalName != elem.path().filename().string())
                {
                    //If it changes to a name that already exist it will crash
                    Dir::ChangeDirectoryLineNumber(elem, dirLineNumber + repetionNumber);
                }
            });
        }
    }
    currentLine += 1;

    lastCommand = Command::Line;
    // std::cout << "Parent: " << basedPath.parent_path().string() << '\n';
    // std::cout << "Create " << finalText << " at " << lineNumber << "\n";
}

void
CommandHandler::SetCurrentLine()
{
    uint64_t line;
    auto numberOfLines = Dir::GetNumberOfDirs();
    switch (arg.c) 
    {
        case 1:
        {
            if (line == numberOfLines) return;
            line = numberOfLines + 1;
        } break;
        case 2:
        {
            line = std::stoll(arg.v.at(1));
        } break;
        default: WrongUsage(Command::SetLine); return;
    }

    if (line < 1 or line > numberOfLines + 1)
    {
        WrongUsage(Command::SetLine);
        std::cerr << "Line Number is out of range\n";
        return;
    }

    currentLine = line;
}

void
CommandHandler::HandleLineDelete()
{
    std::string lineNumberArg       = arg.c > 1? arg.v.at(1) : std::to_string(currentLine);
    uint64_t    referenceLineNumber = 0;
    const auto  numberOfDirs        = Dir::GetNumberOfDirs();

    if (numberOfDirs == 0)
    {
        std::cout << "Already empty\n";
        return;
    }

    lineNumberPtr = &lineNumberArg;
    if ((isRanged = ParseRange(Command::Delete)))
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
            if (currentLine > numberOfDirs)
            {
                std::cerr << "Can't delete at current line: directory at line " << currentLine << " is non-existant\n";
                return;
            }

            referenceLineNumber = currentLine;
            dir = Dir::DirectoryInLine(currentLine); 
            currentLine -= currentLine > 1;
        } break;
        case 2:
        {
            if (isRanged) break;
            if (!zkb::IsInteger(lineNumberArg))
            {
                std::cerr << "Not passing an integer for line number\n";
                return;
            }

            referenceLineNumber = std::stoll(lineNumberArg);
            dir = Dir::DirectoryInLine(referenceLineNumber);
            std::cerr << "Dir in line is " << dir.path().filename().string() << '\n';
        } break;
        default: WrongUsage(Command::Delete); return;
    }

    for (const auto& elem : dirs)
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
    GenericDirectoryIteration([&](zkb::DirEntry elem, uint64_t lineNumber)
    {
        if (lineNumber > (isRanged? upperBound : referenceLineNumber))
        {
            Dir::ChangeDirectoryLineNumber(elem, lineNumber - (upperBound - lowerBound + isRanged));
        }
    });
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
            if ((isRanged = ParseRange(Command::Change)))
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
CommandHandler::HandleLineSwap()
{
    uint64_t targetLine{};
    uint64_t sourceLine{};
    auto numberOfLines = Dir::GetNumberOfDirs();

    lineNumberPtr = &arg.v.at(1);
    if ((isRanged = ParseRange(Command::Swap)))
    {
        if (range.text.at(0) == "-1") return;
    }

    if (!zkb::IsInteger(arg.v.at(1 + isRanged)))
    {
        WrongUsage(Command::Swap);
        std::cerr << "Not passing a number for source\n";
        return;
    }

    const auto& lowerBound = range.num.at(0);
    const auto& upperBound = range.num.at(1);

    switch (arg.c)
    {
        case 2:
        {
            sourceLine = std::stoll(arg.v.at(1));
            if (currentLine == sourceLine) return;

            targetLine = currentLine;

        } break;
        case 3:
        {
            targetLine = std::stoll(arg.v.at(2));
            if (!isRanged)
                sourceLine = std::stoll(arg.v.at(1));
        } break;
        default: WrongUsage(Command::Swap); return;
    }

    //If not ranged then this will never be true
    if (targetLine * !isRanged > numberOfLines)
    {
        WrongUsage(Command::Swap);
        std::cerr << "Target line is out of range\n";
        return;
    }

    if (isRanged)
    {
        if (lowerBound == targetLine) return;
        int64_t numberOfLinesToShift = targetLine - lowerBound;

        std::vector<fs::directory_entry> movingDirs;
        if (numberOfLinesToShift > 0)
        {
            if (upperBound + numberOfLinesToShift > numberOfLines)
            {
                std::cout << "Can't fit! Create new lines to accomodate for this? y/n: ";

                char response;
                std::cin >> response;

                if (std::tolower(response) == 'y')
                {
                    auto tempCurrentLine = currentLine;
                    currentLine = numberOfLines + 1;

                    arg.c = 1;
                    arg.v.at(0) = "l";
                    for (uint64_t i = (upperBound + numberOfLinesToShift) - numberOfLines; i > 0; i -= 1)
                        HandleNewLine();

                    currentLine = tempCurrentLine;
                }
                else
                {
                    return;
                }
            }

            movingDirs = Dir::DirectoriesInRange(upperBound + 1, upperBound + numberOfLinesToShift);
        }
        else
        {
            movingDirs = Dir::DirectoriesInRange(lowerBound + numberOfLinesToShift, lowerBound - 1);
        }

        auto dirs = Dir::DirectoriesInRange(lowerBound, upperBound);
        for (const auto& dir : dirs)
        {
            uint64_t lineNumber = Dir::GetDirectoryLineNumber(dir);
            Dir::ChangeDirectoryLineNumber(dir, lineNumber + numberOfLinesToShift);
        }

        for (const auto& dir : movingDirs)
        {
            Dir::ChangeDirectoryLineNumber(dir, Dir::GetDirectoryLineNumber(dir) - dirs.size() * zkb::Sign(numberOfLinesToShift));
        }
    }
    else
    {
        //Could accomodate this too
        if (sourceLine > Dir::GetNumberOfDirs())
        {
            WrongUsage(Command::Swap);
            std::cerr << "Source line is out of range\n";
            return;
        }

        auto [currentDir, sourceDir] = Dir::DirectoriesInLines(std::make_pair(targetLine, sourceLine));
        // auto sourceDir  = Dir::DirectoryInLine(sourceLine);
        auto tempLineNumber   = Dir::GetDirectoryLineNumber(currentDir);

        Dir::ChangeDirectoryLineNumber(currentDir, Dir::GetDirectoryLineNumber(sourceDir));
        Dir::ChangeDirectoryLineNumber(sourceDir,  tempLineNumber);
    }
    lastCommand = Command::Swap;
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
    switch (arg.c)
    {
        case 1:
        {
            uint64_t lineNumber = 1;
            if (fs::is_empty(basedPath))
            {
                std::cout << "\t\t\t\tNumber of lines: 0" << '\n';
                return;
            }

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
            std::cout << "\t\t\t\t"; ShowStatus();
        } break;
        case 2:
        {
            if (ParseStringText(arg.v.at(1)))
            {
                if (range.text.at(0) == "-1") return;
            }

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
CommandHandler::ChangeDirectory()
{
    fs::path path;
    switch (arg.c)
    {
        case 1:
        {
            
        } break;
        case 2:
        {
            if (zkb::IsInteger(arg.v.at(1)))
            {
                path = Dir::DirectoryInLine(std::stoll(arg.v.at(1)));
                break;
            }

            if (path.string().at(1) != ':' && !path.string().starts_with('.'))
            {
                std::cerr << "Path doesn't exist\n";
                return;
            }

            path = arg.v.at(1);
        } break;
        default: WrongUsage(Command::CD); return;
    }

    fs::current_path(path);
    basedPath = path;
    currentLine = 1;
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
        if constexpr (DEBUG_BUILD)
            std::cerr << "Can't parse string\n";
        return false;
    }

    //TODO: Implement allowing for space on the start of the string. Ex. " my text"
    if (textArg.size() < 2)
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
CommandHandler::ParseRange(Command command)
{
    const auto& string = *lineNumberPtr;
    if (!string.starts_with('(') or !string.ends_with(')'))
    {
        range.num.at(0) = 0;
        range.num.at(1) = 1;
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
        WrongUsage(command);
        if (command == Command::Delete)
        {
            std::cerr << "Use -d to delete a range of lines\n";
        }
        else if (command == Command::Change)
        {
            std::cerr << "Use -c to change a range of lines\n";
        }
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

    if constexpr (DEBUG_BUILD)
    {
        for (const auto& str : range.text)
        {
            std::cerr << "range: " << str << '\n';
        }
    }

    for (int i = 0; i < 2; i += 1)
    {
        // std::cerr << "inside\n";
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
void
CommandHandler::GenericDirectoryIteration(std::function<void(const std::filesystem::directory_entry&, uint64_t)> func)
{
    for (const auto& elem : fs::directory_iterator{basedPath})
    {
        if (!fs::is_directory(elem)) continue;

        func(elem, Dir::GetDirectoryLineNumber(elem));
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
