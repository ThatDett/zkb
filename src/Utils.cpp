#include <cstdint>
#include <iostream>
#include <string>
#include <array>

#include "Utils.hpp"
#include "Directory.hpp"
#include "CommandHandler.hpp"

bool zkb::IsInteger(const std::string& str)
{
    for (const unsigned char ch : str)
    {
        if (!std::isdigit(ch)) return false;
    }

    return true;
}

void zkb::ToLower(std::string& string)
{
    for (char& ch : string) ch = std::tolower(ch);
}

std::array<std::string, 2>
zkb::ParseRange(const std::string& string)
{
    uint32_t startPos = 0;
    std::array<std::string, 2> rangeParams;

    for (const char& ch : string)
    {
        if (std::isdigit(ch)) break;
        if (ch == ',') 
        {
            startPos = 0;
            break;
        }

        startPos += 1;
    }

    if (string.ends_with(')'))
    {
        auto commaPos = string.find(',');
        
        rangeParams.at(0) = startPos > 0? string.substr(startPos, commaPos - 1) : "1";

        for (const char& ch : string.substr(commaPos + 1, string.size() - 1))
        {
            if (std::isdigit(ch)) break;
            if (ch == ')') 
            {
                commaPos = 0;
            }
        }

        rangeParams.at(1) = commaPos > 0? string.substr(commaPos + 1, string.size() - 1) : std::to_string(Directory::GetNumberOfDirs());

        for (const auto& str : rangeParams)
        {
            std::cerr << "range: " << str << '\n';
        }

        return rangeParams;
    }
    
    using CH = CommandHandler;
    CH::WrongUsage(CH::Command::Remove);
    return {"-1"};
}
