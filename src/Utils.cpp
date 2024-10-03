#include <cctype>
#include <string>

#include "Utils.hpp"

bool
zkb::IsInteger(const std::string& str)
{
    for (const unsigned char& ch : str)
    {
        if (!std::isdigit(ch)) return false;
    }

    return true;
}

void
zkb::ToLower(std::string& string)
{
    for (char& ch : string) ch = std::tolower(ch);
}

std::string
zkb::ToLower(std::string&& string)
{
    std::string out = std::move(string);
    ToLower(out);
    return out;
}

int 
zkb::Sign(int num)
{
    if (num == 0) return 0;
    return num > 0? 1 : -1;
}
