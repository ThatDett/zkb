#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include "CommandHandler.hpp"
#include <cstdint>
#include <span>
#include <stack>
#include <string>
#include <filesystem>

namespace zkb
{
    namespace fs   = std::filesystem;

    class Directory
    {
    public:
        Directory();
        Directory(fs::directory_entry);

    public:
        void Name(const std::string&);
        auto Name() const
          -> const std::string&;

        void LineNumber(uint32_t);
        auto LineNumber() const
          -> uint32_t;

        void Filename(const std::string&);
        auto Filename() const
          -> std::string;

        bool IsInitialized() const;

        auto operator()() const
          -> fs::directory_entry;
        void operator()(uint32_t lineNumber);
    private:
        fs::directory_entry directoryEntry;

        uint32_t    lineNumber         = 0;
        std::string name;
        bool        alreadyInitialized = false;

    public:

    public:
        static auto DirectoryInLine(uint32_t, std::span<const char*> namesToAvoid = std::span<const char*>{}) 
            -> fs::directory_entry;

        static auto DirectoriesInLines(const std::pair<uint32_t, uint32_t>&) 
            -> std::pair<fs::directory_entry, fs::directory_entry>;

        static auto DirectoriesInRange(uint32_t upperBound, uint32_t lowerBound) 
            -> std::vector<fs::directory_entry>;

        static auto GetDirectoryLineNumber(const fs::path& = fs::current_path()) 
            -> uint32_t;
        static auto GetDirectoryName(const fs::path& = fs::current_path())       
            -> std::string;
        static auto GetNumberOfDirs() 
            -> uint32_t;

        static auto ChangeDirectoryLineNumber(const fs::directory_entry&, uint32_t offset, bool ignoreError = false)
            -> fs::directory_entry;
        static auto ChangeDirectoryName(const fs::directory_entry&, const std::string&)
            -> fs::directory_entry;
        static auto ChangeDirectoryFilename(const fs::directory_entry&, const std::string&)
            -> fs::directory_entry;

        static void RecursivelyDelete(const fs::directory_entry&, bool save = true);

        static auto CreateDirectory(const std::string&, const fs::path& = fs::current_path())
            -> bool;
        static auto RemoveDirectory(const fs::directory_entry&)
            -> bool;

        static auto PathIterator(fs::path = CommandHandler::basedPath)
            -> fs::directory_iterator;
        
    public:
        static std::stack<CommandHandler::HistoryT> history;

        static uint32_t numberOfDirs;
        static bool     updateNumberOfDirs;
    };
}

#endif
