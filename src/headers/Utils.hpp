#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <array>

namespace zkb
{
    bool IsInteger(const std::string&);
    void ToLower(std::string&);
    auto ParseRange(const std::string&) -> std::array<std::string, 2>;
}

#endif
