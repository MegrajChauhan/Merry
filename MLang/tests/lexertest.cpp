#include "../includes/lexer.hpp"

using namespace mlang;

int main()
{
    Lexer lexer;
    if (lexer.setup_reader("../examples/test.ml") == false)
    {
        return -1;
    }
    auto lexed = lexer.lex();
    while (lexed.type != _TT_EOF)
    {
        std::cout << "TYPE: " << lexed.type << "\nVALUE: " << lexed.value << "\n\n";
        lexed = lexer.lex();
    }
}