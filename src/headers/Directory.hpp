#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <cstdint>
#include <string>
#include <filesystem>

namespace zkb
{
    namespace fs = std::filesystem;
    using String = const std::string&;
    using Path   = const fs::path&;

    class Directory
    {
    public:
        static auto DirectoryInLine(uint64_t) 
            -> fs::directory_entry;
 
        static auto GetDirectoryLineNumber(Path = fs::current_path()) -> uint64_t;
        static auto GetDirectoryName(Path = fs::current_path())       -> std::string;
        static void ChangeDirectoryLineNumber(const fs::directory_entry& dir, uint64_t offset);

        static auto CreateDirectory(String)
            -> bool;

        static auto GetNumberOfDirs() 
            -> uint64_t;
        
        static auto PathIterator()
            -> fs::directory_iterator const;

    };
}

#endif
