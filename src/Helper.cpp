#include <iostream>

#include "Helper.hpp"

void zkb::Helper::RootDirectory()
{
    std::cout <<
    "The project must have a root directory. The root directory"
    " must end with a .zkb suffix\nand the current directory when the tooling"
    " is called must be the root or a child. \n\nExample: Project.zkb\n"
    "    Project.zkb\n"
    "    |- 1 IsEven(num [int]) [bool]\n"
    "       |- 1 return a % 2 == 0\n"
    "    |- 2 main() [int]\n"
    "       |- 1 print('Hello, World\\n')\n"
    "       |- 2 println('3 is even?')\n"
    "       |- 3 println(IsEven(3))\n"
    "       |- 4 return 0\n\n";
}
