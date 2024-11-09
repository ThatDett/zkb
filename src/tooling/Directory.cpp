#include <cstdint>
#include <filesystem>
#include <iostream>
#include <span>
#include <stack>
#include <string>
#include <utility>

#include "Directory.hpp"
#include "CommandHandler.hpp"

using Directory = zkb::Directory;
namespace fs = zkb::fs;

std::stack<CommandHandler::HistoryT> Directory::history = {};

Directory::Directory() {};

Directory::Directory(fs::directory_entry dirEntry) :
    directoryEntry(std::move(dirEntry)),
    lineNumber(GetDirectoryLineNumber(directoryEntry)),
    name(GetDirectoryName(directoryEntry)),
    alreadyInitialized(true)
{
    // std::cout << directoryEntry.path().string() << '\n';
}

void
Directory::Name(const std::string& newName)
{
    directoryEntry = ChangeDirectoryName(directoryEntry, newName);
    this->name     = newName;
}

const std::string&
Directory::Name() const
{
    return name;
}

void
Directory::LineNumber(uint32_t lineNumber)
{
    directoryEntry   = ChangeDirectoryLineNumber(directoryEntry, lineNumber);
    this->lineNumber = lineNumber;
}

uint32_t
Directory::LineNumber() const
{
    return lineNumber;
}

void
Directory::Filename(const std::string& newFileName)
{
    directoryEntry = ChangeDirectoryFilename(directoryEntry, newFileName);
}

std::string
Directory::Filename() const
{
    return directoryEntry.path().filename().string();
}

fs::directory_entry
Directory::operator()() const
{
    return directoryEntry;
}

void
Directory::operator()(uint32_t lineNumber)
{
    if (alreadyInitialized)
    {
        std::cout << "Directory " << name << " already initialized.\n";
        return;
    }

    directoryEntry   = DirectoryInLine(lineNumber);
    this->lineNumber = lineNumber;
    name             = GetDirectoryName(directoryEntry);
}

bool
Directory::IsInitialized() const
{
    return alreadyInitialized;
}

auto Directory::DirectoryInLine(uint32_t lineNumber, std::span<const char*> namesToAvoid) -> fs::directory_entry
{
    fs::directory_entry dirInLine;
    for (const auto& elem : PathIterator())
    {
        if (!elem.is_directory()) break;
        for (const auto& nameToAvoid : namesToAvoid)
        {
            if (GetDirectoryName(elem) == nameToAvoid)
            {
                continue;
            }
        }

        if (GetDirectoryLineNumber(elem) == lineNumber)
        {
            const auto name = GetDirectoryName(elem);
            if (name.ends_with("temp"))
            {
                return elem;
            }
            else
            {
                dirInLine = elem;
            }
        }
    }

    if (dirInLine.exists())
        return dirInLine;

    std::cerr << "Acessing non-existant line number " << lineNumber <<
    " in "  << fs::current_path().string() << "\nThis path has "    << 
    GetNumberOfDirs() << " directories.\n";

    return fs::directory_entry{"null"};
}

std::pair<fs::directory_entry, fs::directory_entry>
Directory::DirectoriesInLines(const std::pair<uint32_t, uint32_t>& lines) 
{
    const auto& firstLine  = lines.first;
    const auto& secondLine = lines.second;

    fs::directory_entry first;
    fs::directory_entry second;

    for (auto& elem : PathIterator())
    {
        if (!elem.is_directory()) break;

        uint32_t dirLineNumber = GetDirectoryLineNumber(elem);
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
Directory::DirectoriesInRange(uint32_t lowerBound, uint32_t upperBound) 
{
    std::vector<fs::directory_entry> dirs;
    dirs.reserve(upperBound - lowerBound + 1);

    for (auto& elem : PathIterator())
    {
        if (!elem.is_directory()) break;

        uint32_t dirLineNumber = GetDirectoryLineNumber(elem);
        if (dirLineNumber >= lowerBound and dirLineNumber <= upperBound)
        {
            dirs.emplace_back(std::move(elem));
        }
    }

    return dirs;
}

bool
Directory::CreateDirectory(const std::string& name, const fs::path& path)
{
    const fs::path _path = path.string() + '\\' + name;
    std::cerr << "Creating " << name /*<< " as " << _path.string()*/ << "\n\n";
    numberOfDirs += 1;
    return fs::create_directory(_path);
}

bool
Directory::RemoveDirectory(const fs::directory_entry& dir)
{
    const auto& lineNumber = std::to_string(GetDirectoryLineNumber(dir));
    auto filename   = GetDirectoryName(dir);

    history.push(CommandHandler::HistoryT
        {
            {{"l", filename, lineNumber}, 3},
            fs::current_path()
        }
    );

    numberOfDirs -= 1;
    return fs::remove(dir);
}

void
Directory::RecursivelyDelete(const fs::directory_entry& dir, bool save)
{
    auto Delete = [dir]()
    {
        const auto& lineNumber = std::to_string(GetDirectoryLineNumber(dir));
        const auto& filename   = GetDirectoryName(dir);

        std::cerr << "Pushing " << dir.path() << '\n';
        std::cerr << "remove " << dir.path().string() << "\n";
        std::cerr << "My parent: " << dir.path().parent_path() << "\n\n";

        history.push(CommandHandler::HistoryT
            {
                {{"l", filename, lineNumber}, 3},
                dir.path().parent_path()
            }
        );

        numberOfDirs -= 1;
        fs::remove(dir);
    };

    if (fs::is_empty(dir))
    {
        if (save) 
        {
            Delete();
        }
        else
        {
            numberOfDirs -= 1;
            fs::remove(dir);
        }
        return;
    }
    
    for (const auto& elem : fs::recursive_directory_iterator{dir})
    {
        RecursivelyDelete(elem);
    }

    //Delete parent
    if (save) 
    {
        Delete();
    }
    else
    {
        numberOfDirs -= 1;
        fs::remove(dir);
    }
}

bool     Directory::updateNumberOfDirs = true;
uint32_t Directory::numberOfDirs       = 0;

uint32_t 
Directory::GetNumberOfDirs()
{
    if (updateNumberOfDirs)
    {
        // std::cerr << "Updating number of dirs\n";
        numberOfDirs = 0;
        for (const auto& elem : PathIterator())
        {
            if (elem.is_directory()) numberOfDirs += 1;
        }
        updateNumberOfDirs = false;
    }


    return numberOfDirs;
}

uint32_t
Directory::GetDirectoryLineNumber(const fs::path& path)
{
    const std::string dir = path.filename().string();
    auto lineNumber = std::stoi(dir.substr(0, dir.find_first_of(' ')));
    return lineNumber;
}

std::string
Directory::GetDirectoryName(const fs::path& path)
{
    const std::string dir = path.filename().string();
    return dir.substr(dir.find_first_of(' ') + 1, dir.size());
}

fs::directory_entry
Directory::ChangeDirectoryLineNumber(const fs::directory_entry& elem, uint32_t number, bool ignoreError)
{
    auto fullDirName = std::to_string(number) + ' ' + GetDirectoryName(elem.path());
    auto path        = CommandHandler::basedPath.string() + "\\" + fullDirName;
    
    // if (ignoreError)
    // {
        std::error_code err;
        // fs::rename(elem, path, err);
    // }
    // else
    // {
        fs::rename(elem, path, err);
        // std::cout << err.message() << '\n';
    // }
    return fs::directory_entry(std::move(path));
}

fs::directory_entry
Directory::ChangeDirectoryName(const fs::directory_entry& elem, const std::string& newName)
{
    auto fullDirName = std::to_string(GetDirectoryLineNumber(elem)) + ' ' + newName;
    auto path        = CommandHandler::basedPath.string() + "\\" + fullDirName;
    
    fs::rename(elem, path);
    return fs::directory_entry(std::move(path));
}

fs::directory_entry
Directory::ChangeDirectoryFilename(const fs::directory_entry& elem, const std::string& newName)
{
    auto path = elem.path().parent_path().string() + "\\" + newName;
    
    fs::rename(elem, path);
    return fs::directory_entry(std::move(path));
}

auto
Directory::PathIterator(fs::path path) -> fs::directory_iterator
{
    return fs::directory_iterator{ path };
}
