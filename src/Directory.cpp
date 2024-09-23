#include <filesystem>
#include <unordered_map>

#include "Directory.hpp"

using Directory = zkb::Directory;
namespace fs = zkb::fs;

static std::unordered_map<std::string, Directory> directoryMap;

auto Directory::DirectoryInLine(const std::string& path, uint64_t index) -> Directory&
{
    return directoryMap.at(path + std::to_string(index));
}

bool
Directory::CreateDirectory(const std::string& name)
{
    auto& parent = GetDirInfo();

    parent.numberOfDirs += 1;
    directoryMap.insert(std::make_pair(
                            parent.path.string() + name.substr(0, name.find(' ')),
                            Directory{
                                .numberOfDirs = 0,
                                .path = fs::path(parent.path.string() + name)
                            }
                        ));
                            
    return fs::create_directory(name);
}

uint64_t 
Directory::GetNumberOfDirs()
{
    if (fs::is_empty(fs::current_path())) return 0;
    int numberOfDirs = 0;

    for (const auto& _ : PathIterator())
    {
        (void)_;
        numberOfDirs += 1;            
    }
    return numberOfDirs;
}

auto
Directory::GetDirInfo(const fs::path& path) -> zkb::Directory&
{
    if (!directoryMap.contains(path))
    {
        directoryMap.insert(std::make_pair(
                                path, 
                                zkb::Directory{
                                    .numberOfDirs = GetNumberOfDirs(path),
                                    .lineNumber   = 0
                                 }
                             )
                         );
    }
    return directoryMap.at(path);
}

auto
Directory::PathIterator() -> fs::directory_iterator const
{
    return fs::directory_iterator{ fs::current_path() };
}
