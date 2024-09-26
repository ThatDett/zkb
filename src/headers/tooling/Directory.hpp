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
 
        constexpr static auto GetDirectoryLineNumber(Path = fs::current_path()) 
            -> uint64_t;
        constexpr static auto GetDirectoryName(Path = fs::current_path())       
            -> std::string;
        static auto GetNumberOfDirs() 
            -> uint64_t;

        static void ChangeDirectoryLineNumber(DirEntry, uint64_t offset);
        static void RecursivelyDelete(DirEntry, bool save = true);

        static auto CreateDirectory(String, fs::path = fs::current_path())
            -> bool;
        static auto RemoveDirectory(DirEntry)
            -> bool;

        static auto PathIterator(fs::path = fs::current_path())
            -> fs::directory_iterator;
        
    public:
        static std::stack<HistoryT> history;
    };
}

#endif
