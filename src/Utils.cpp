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


