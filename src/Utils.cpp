
#include "Utils.hpp"

bool zkb::IsInteger(const std::string& str)
{
    for (const char& ch : str)
    {
        if (!std::isdigit(ch)) return false;
    }

    return true;
}
