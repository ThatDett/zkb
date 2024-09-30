#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <string_view>

namespace zkb
{
    bool IsInteger(const std::string&);
    void ToLower(std::string&);
    std::string ToLower(const std::string&);
}

#endif
