#include "../includes/lexer.hpp"

bool mlang::Lexer::setup_reader(std::string filename)
{
    reader.set_filename(filename);
    return reader.open_and_setup();
}

mlang::Token mlang::Lexer::lex()
{
    Token token;
    std::string val;

    
}