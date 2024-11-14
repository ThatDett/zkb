#ifndef APPLICATION_H
#define APPLICATION_H

#include <cstdlib>
#include <cstdint>

#include <string_view>

class Application
{
public:
    Application(char** argv, int argc);

    void Build();

    static void PrintHelp(const std::string_view);
    static void Exit(uint32_t errorCode = EXIT_SUCCESS);

private:
    char** argv;
    int    argc;
};
#endif
