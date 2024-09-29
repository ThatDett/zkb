#include <vector>
#include <iostream>
#include <string_view>

#include "Application.hpp"
#include "CommandHandler.hpp"
#include "Helper.hpp"

namespace fs = std::filesystem;

std::vector<std::string> keywords
{
    "project", "root"
};

Application::Application(char** _argv, int _argc) :
    argv(_argv), argc(_argc)
{
    const auto getSomeHelp = [&]()
    {
        std::cout << "\nUse: \"zkb help (keyword)\" for project/compiler help\n"
        "Run the tooling with: \"zkb\" and use: \"help\" for tooling help\n" 
        "keywords:\n";

        for (const auto& str : keywords)
        {
            std::cout << "  \"" << str << "\"\n";
        }

        std::cout << "For more info see the repo: https://github.com/ThatDett/zkb\n"
        "\n"
        ;
    };

    if (argc == 1)
    {
        //Run tooling
        CommandHandler();
        std::cout << "Exiting...\n";
        return;
    }
    
    const std::string first = argv[1];
    if (first == "build")
    {
        Build();
    }
    else if (first == "help")
    {
        if (argc != 3)
        {
            getSomeHelp();
        }

        PrintHelp(argv[2]);
    }
    else 
    {
        getSomeHelp();
    }
}

void Application::Build()
{
    std::cerr << "Not implemented.\n";
}

void Application::PrintHelp(const std::string_view str)
{
    if (str == "project")
    {
        std::cout << "\nIt is recommended to add the executable to the system's path\n\n"
        "Start first by creating a new project folder with the \".zkb\" suffix\n"
        "Change your directory into that folder and run the tooling with \"zkb\" and no more args\n"
        "Run \"help\" in the tooling for help about zkb tool's commands\n"
        "\n"
        ;
    }
    else if (str == "root")
    {
        zkb::Helper::RootDirectory();
    }
}
