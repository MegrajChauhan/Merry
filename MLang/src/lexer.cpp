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
    if (std::isspace(*curr_char))
    {
        clear_unnecessary();
    }
    if (*curr_char == '/' && peek() == '/')
    {
        // this is a comment and we can completely skip this
        consume_comment();
    }
    if (eof)
        return Token(_TT_EOF, "");
    if (is_oper(*curr_char))
    {
        val.push_back(*curr_char);
        consume();
        // we don't have multi-character operators and so we can skip it as well
        auto operkind = _iden_map_.find(val);
        // operkind should be valid
        token.type = (*operkind).second;
    }
    if (is_alpha(*curr_char))
    {
    }
}

void mlang::Lexer::invalid_token()
{
    std::cerr << "While " << _CCODE_BOLD << "Lexing,\n";
    std::cerr << reader.get_file_name() << ":" << reader.get_current_line_num() << ":" << col_no << ":" << _CCODE_RESET;
    std::cerr << "Invalid token received: \n"; // we could add "expected" to make the error more informative
    

    exit(EXIT_FAILURE); // this is a failure
}