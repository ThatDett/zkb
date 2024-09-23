#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <cstdint>
#include <string>
#include <filesystem>
#include <unordered_map>

namespace zkb
{
    namespace fs = std::filesystem;
    class Directory
    {
    public:
        auto DirectoryInLine(uint64_t) -> Directory&;

        static bool CreateDirectory(const std::string&);
        static auto GetNumberOfDirs() -> uint64_t;
        static auto PathIterator()    -> zkb::fs::directory_iterator const;
        static auto GetDirInfo(const fs::path& str = fs::current_path()) -> Directory&;

        uint64_t numberOfDirs = 0;
        uint64_t lineNumber   = 0;
        fs::path path;

        static std::unordered_map<fs::path, Directory> directoryMap;
    };
}

#endif
