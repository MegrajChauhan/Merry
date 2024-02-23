#include "../includes/parser.hpp"

masm::parser::Parser::Parser(masm::lexer::Lexer lexer)
{
    this->lexer = lexer; // the lexer needs to be initialized here
}

void masm::parser::Parser::setup_lexer(std::string filepath)
{
    lexer.setup_reader(filepath);
}