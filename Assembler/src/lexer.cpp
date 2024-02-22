#include "../includes/lexer.hpp"

bool masm::lexer::Lexer::setup_reader(std::string filename)
{
    masm::reader::Reader reader(filename);
    reader.setup();
    file_contents = reader.read();
    curr_char = file_contents.begin();
    this->filename = filename;
    return true;
}

masm::lexer::Token::Token(masm::lexer::TokenType type, std::string val)
{
    this->type = type;
    this->value = val;
}

masm::lexer::Token masm::lexer::Lexer::lex()
{
    Token token;
    std::string val;
    if (std::isspace(*curr_char) || *curr_char == '\n')
    {
        clear_unnecessary();
    }
    if (*curr_char == ';' && peek() == ';')
    {
        // this is a comment and we can completely skip this
        while (*curr_char == ';' && peek() == ';')
            consume_comment();
    }
    if (curr_char == file_contents.end())
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
    else if (is_alpha(*curr_char) || *curr_char == '.')
    {
        if (*curr_char == '.')
        {
            consume();
        }
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

void masm::lexer::Lexer::invalid_token(std::string read)
{
    std::cerr << "While " << _CCODE_BOLD << "Lexing:\n";
    std::cerr << std::filesystem::path(std::filesystem::current_path() / filename) << ":" << line_num + 1 << ":" << col_no << ":" << _CCODE_RESET;
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

void masm::lexer::Lexer::invalid_token()
{
    std::cerr << "While " << _CCODE_BOLD << "Lexing:\n";
    std::cerr << std::filesystem::path(std::filesystem::current_path() / filename) << ":" << line_num + 1 << ":" << col_no << ":" << _CCODE_RESET;
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

std::vector<masm::lexer::Token> masm::lexer::Lexer::lex_all()
{
    std::vector<Token> alltoks;
    Token token;
    std::string val;
    while (true)
    {
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
        if (curr_char == file_contents.end())
        {
            alltoks.push_back(Token(_TT_EOF, ""));
            break;
        }
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
        alltoks.push_back(token);
        val.clear();
    }
    return alltoks;
}