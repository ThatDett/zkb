#ifndef APPLICATION_H
#define APPLICATION_H

#include <string_view>

class Application
{
public:
    Application(char** argv, int argc);

    void Build();
    static void PrintHelp(const std::string_view);

private:
    char** argv;
    int    argc;
};
#endif
