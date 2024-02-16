#include "../includes/lexer.hpp"

bool mlang::Lexer::setup_reader(std::string filename)
{
    Reader reader(filename);
    if (!reader.setup())
        return false;
    file_contents = reader.read();
    curr_char = file_contents.begin();
    this->filename = filename;
    return true;
}

mlang::Token::Token(mlang::TokenType type, std::string val)
{
    this->type = type;
    this->value = val;
}

mlang::Token mlang::Lexer::lex()
{
    Token token;
    std::string val;
    if (std::isspace(*curr_char) || *curr_char == '\n')
    {
        clear_unnecessary();
    }
    if (*curr_char == '/' && peek() == '/')
    {
        // this is a comment and we can completely skip this
        while (*curr_char == '/' && peek() == '/')
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
    else if (is_alpha(*curr_char))
    {
        token = get_iden_or_keyword();
    }
    else if (is_num(*curr_char))
    {
        token = get_number();
    }
    else
    {
        invalid_token();
    }
    return token;
}

void mlang::Lexer::invalid_token(std::string read)
{
    std::cerr << "While " << _CCODE_BOLD << "Lexing:\n";
    std::cerr << std::filesystem::path(std::filesystem::current_path() / filename) << ":" << line_num << ":" << col_no << ":" << _CCODE_RESET;
    std::cerr << " Invalid token received: \n"; // we could add "expected" to make the error more informative
    // PosDetail detail = extract_word(current_line, curr_char);
    std::cerr << "  " << line_num + 1 << "| " << get_current_line() << "\n";
    std::cerr << "    ";
    for (size_t i = 0; i <= (col_no - read.length()); i++)
    {
        std::cerr << " ";
    }
    curr_char -= read.length();
    while (*curr_char != ' ' && curr_char != file_contents.end())
    {
        std::cerr << "^";
        curr_char++;
    }
    std::cerr << "\nAborting further compilation." << std::endl;
    exit(EXIT_FAILURE); // this is a failure
}

void mlang::Lexer::invalid_token()
{
    std::cerr << "While " << _CCODE_BOLD << "Lexing:\n";
    std::cerr << std::filesystem::path(std::filesystem::current_path() / filename) << ":" << line_num << ":" << col_no << ":" << _CCODE_RESET;
    std::cerr << " Invalid token received: \n"; // we could add "expected" to make the error more informative
    // PosDetail detail = extract_word(current_line, curr_char);
    std::cerr << "  " << line_num + 1 << "| " << get_current_line() << "\n";
    std::cerr << "    ";
    for (size_t i = 0; i <= (col_no); i++)
    {
        std::cerr << " ";
    }
    std::cerr << "^";
    std::cerr << "\nAborting further compilation." << std::endl;
    exit(EXIT_FAILURE); // this is a failure
}