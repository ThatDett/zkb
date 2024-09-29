#include <string>

#include "Utils.hpp"

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


