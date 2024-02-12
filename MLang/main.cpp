#include <iostream>
#include "includes/lexer.hpp"

/// TODO: I leave parsing command line arguments to luccie_cmd

int main(int argc, char **argv)
{
    MLang::Lexer lexer("examples/test.ml");
    if (!lexer.open_file_for_lexing())
    {
        std::cout << "Error\n";
        return -1;
    }
    return 0;
}