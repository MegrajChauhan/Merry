#include <iostream>
#include "includes/lexer.hpp"
// #include "lexer.hpp"

/// TODO: I leave parsing command line arguments to luccie_cmd

int main(int argc, char **argv)
{
    MLang::Lexer lexer("examples/test.ml");
    if (!lexer.open_file_for_lexing())
    {
        std::cout << "Error\n";
        return -1;
    }
    // now lets see if everything is lexed correctly
    auto lexed = lexer.lex();
    while (lexed.type != MLang::_TT_EOF)
    {
        std::cout << "Token type: " << lexed.type << "\nValue: " << lexed.value << "\n";
        lexed = lexer.lex();
    }
    return 0;
}