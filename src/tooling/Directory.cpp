#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stack>
#include <string>
#include <utility>

#include "Directory.hpp"
#include "CommandHandler.hpp"

using Directory = zkb::Directory;
using String    = zkb::String;
namespace fs = zkb::fs;

std::stack<Directory::HistoryT> Directory::history = {};

auto Directory::DirectoryInLine(uint64_t lineNumber) -> fs::directory_entry
{
    for (const auto& elem : PathIterator())
    {
        if (!elem.is_directory()) break;
        if (GetDirectoryLineNumber(elem) == lineNumber) return elem;
    }

    std::cerr << "Acessing non-existant line number " << lineNumber <<
    " in "  << fs::current_path().string() << "\nThis path has "    << 
    GetNumberOfDirs() << " directories.\n";

    return fs::directory_entry{"null"};
}

std::pair<fs::directory_entry, fs::directory_entry>
Directory::DirectoriesInLines(const std::pair<uint64_t, uint64_t>& lines) 
{
    const auto& firstLine  = lines.first;
    const auto& secondLine = lines.second;

    fs::directory_entry first;
    fs::directory_entry second;

    for (auto& elem : PathIterator())
    {
        if (!elem.is_directory()) break;

        uint64_t dirLineNumber = GetDirectoryLineNumber(elem);
        if (dirLineNumber == firstLine)
        {
            first = std::move(elem);
        }

        if (dirLineNumber == secondLine)
        {
            second = std::move(elem);
        }

        if (first.exists() and second.exists())
        {
            return std::make_pair(std::move(first), std::move(second));
        }
    }

    std::cerr << "Acessing non-existant line numbers " << first <<
    " in "  << fs::current_path().string() << "\nThis path has "    << 
    GetNumberOfDirs() << " directories.\n";

    return {fs::directory_entry{"null"}, fs::directory_entry{"null"}};
}

std::vector<fs::directory_entry>
Directory::DirectoriesInRange(uint64_t lowerBound, uint64_t upperBound) 
{
    std::vector<fs::directory_entry> dirs;
    dirs.reserve(upperBound - lowerBound + 1);

    for (auto& elem : PathIterator())
    {
        if (!elem.is_directory()) break;

        uint64_t dirLineNumber = GetDirectoryLineNumber(elem);
        if (dirLineNumber >= lowerBound and dirLineNumber <= upperBound)
        {
            dirs.emplace_back(std::move(elem));
        }
    }

    return dirs;
}

bool
Directory::CreateDirectory(String name, const fs::path& path)
{
    const fs::path _path = path.string() + '\\' + name;
    std::cerr << "Creating " << name << " as " << _path.string() << " ...\n\n";
    return fs::create_directory(_path);
}

bool
Directory::RemoveDirectory(zkb::DirEntry dir)
{
    const auto& lineNumber = std::to_string(GetDirectoryLineNumber(dir));
    auto filename   = GetDirectoryName(dir);

    history.push(HistoryT
        {
            {{"l", filename, lineNumber}, 3},
            fs::current_path()
        }
    );

    return fs::remove(dir);
}

void
Directory::RecursivelyDelete(zkb::DirEntry dir, bool save)
{
    auto Delete = [dir]()
    {
        const auto& lineNumber = std::to_string(GetDirectoryLineNumber(dir));
        const auto& filename   = GetDirectoryName(dir);

        std::cerr << "Pushing " << dir.path() << '\n';
        std::cerr << "remove " << dir.path().string() << "\n";
        std::cerr << "My parent: " << dir.path().parent_path() << "\n\n";

        history.push(HistoryT
            {
                {{"l", filename, lineNumber}, 3},
                dir.path().parent_path()
            }
        );

        fs::remove(dir);
    };

    if (fs::is_empty(dir))
    {
        if (save) 
            Delete();
        else
            fs::remove(dir);
        return;
    }
    
    for (const auto& elem : fs::recursive_directory_iterator{dir})
    {
        RecursivelyDelete(elem);
    }

    //Delete parent
    if (save) 
        Delete();
    else
        fs::remove(dir);
}

uint64_t 
Directory::GetNumberOfDirs()
{
    uint64_t numberOfDirs = 0;
    for (const auto& elem : PathIterator())
    {
        if (elem.is_directory()) numberOfDirs += 1;
    }

    return numberOfDirs;
}

uint64_t
Directory::GetDirectoryLineNumber(Path path)
{
    String dir = path.filename().string();
    return std::stoi(dir.substr(0, dir.find(' ')));
}

std::string
Directory::GetDirectoryName(Path path)
{
    String dir = path.filename().string();
    return dir.substr(dir.find(' ') + 1, dir.size());
}

void
Directory::ChangeDirectoryLineNumber(DirEntry elem, uint64_t number)
{
    auto fullDirName = std::to_string(number) + ' ' + GetDirectoryName(elem.path());
    auto path        = CommandHandler::basedPath.string() + "\\" + fullDirName;
    
    // std::error_code err;
    fs::rename(elem.path(), std::move(path));
    // if (err)
    // {
    //     std::cerr << "Can't change directory number\n";
    //     return;
    // }
}

void
Directory::ChangeDirectoryName(DirEntry elem, const std::string& newName)
{
    auto fullDirName = std::to_string(GetDirectoryLineNumber(elem)) + ' ' + newName;
    auto path        = CommandHandler::basedPath.string() + "\\" + fullDirName;
    
    fs::rename(elem.path(), std::move(path));
}

auto
Directory::PathIterator(fs::path path) -> fs::directory_iterator
{
    return fs::directory_iterator{ path };
}
