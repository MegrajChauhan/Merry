#include "../includes/lexer.hpp"
#include <iostream>

int main()
{
    masm::lexer::Lexer lexer;
    lexer.setup_reader("test.masm");
    auto lexed = lexer.lex();
    while (lexed.type != masm::lexer::_TT_EOF)
    {
        std::cout << "TYPE: " << lexed.type << " VALUE: " << lexed.value << std::endl;
        lexed = lexer.lex();
    }
}