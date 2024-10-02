#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <cstdint>
#include <stack>
#include <string>
#include <filesystem>

#include "CommandHandler.hpp"

namespace zkb
{
    namespace fs   = std::filesystem;
    using String   = const std::string&;
    using Path     = const fs::path&;
    using DirEntry = const fs::directory_entry&;

    class Directory
    {
    public:
        struct HistoryT
        {
            CommandHandler::ArgT args;
            fs::path             path;
        };

    public:
        static auto DirectoryInLine(uint64_t) 
            -> fs::directory_entry;

        static auto DirectoriesInLines(const std::pair<uint64_t, uint64_t>&) 
            -> std::pair<fs::directory_entry, fs::directory_entry>;

        static auto DirectoriesInRange(uint64_t upperBound, uint64_t lowerBound) 
            -> std::vector<fs::directory_entry>;

        static auto GetDirectoryLineNumber(Path = fs::current_path()) 
            -> uint64_t;
        static auto GetDirectoryName(Path = fs::current_path())       
            -> std::string;
        static auto GetNumberOfDirs() 
            -> uint64_t;

        static void ChangeDirectoryLineNumber(DirEntry, uint64_t offset);
        static void ChangeDirectoryName(DirEntry, const std::string&);

        static void RecursivelyDelete(DirEntry, bool save = true);

        static auto CreateDirectory(String, Path = fs::current_path())
            -> bool;
        static auto RemoveDirectory(DirEntry)
            -> bool;

        static auto PathIterator(fs::path = CommandHandler::basedPath)
            -> fs::directory_iterator;
        
    public:
        static std::stack<HistoryT> history;
    };
}

#endif
