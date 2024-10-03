#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <string_view>

namespace zkb
{
    bool IsInteger(const std::string&);

    void ToLower(std::string&);
    auto ToLower(std::string&&)
      -> std::string;

    int Sign(int);
}

#endif
