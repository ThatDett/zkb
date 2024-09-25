#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>

#include "Directory.hpp"

using Directory = zkb::Directory;
using String    = zkb::String;
namespace fs = zkb::fs;

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

bool
Directory::CreateDirectory(String name)
{
    return fs::create_directory(name);
}

void
Directory::RecursivelyDelete(zkb::DirEntry dir)
{
    if (fs::is_empty(dir))
    {
        // const auto& parent     = dir.path().parent_path();
        // uint64_t    lineNumber; 
        //
        // for (const auto& elem : fs::directory_iterator{parent})
        // {
        //     lineNumber = GetDirectoryLineNumber(elem);
        //     if (lineNumber >= GetDirectoryLineNumber(dir))
        //     {
        //         ChangeDirectoryLineNumber(elem, lineNumber - 1);
        //     }
        // }

        fs::remove(dir);
        return;
    }
    
    for (const auto& elem : fs::recursive_directory_iterator{dir})
    {
        RecursivelyDelete(elem);
    }
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
    return dir.substr(dir.find(' '), dir.size());
}

void
Directory::ChangeDirectoryLineNumber(const fs::directory_entry& elem, uint64_t number)
{
    if (!elem.is_directory()) return;
    const auto& fullDirName = std::to_string(number) + GetDirectoryName(elem.path());
    const auto& path        = fs::current_path().string() + "\\" + fullDirName;

    std::error_code err;
    fs::rename(elem.path(), path, err);
    if (err)
    {
        std::cerr << "Can't change directory number\n";
    }
}

auto
Directory::PathIterator() -> fs::directory_iterator const
{
    return fs::directory_iterator{ fs::current_path() };
}
