#include "../includes/lexer.hpp"

// bool masm::lexer::Lexer::setup_reader(std::string filename)
// {
//     masm::reader::Reader reader(filename);
//     reader.setup();
//     file_contents = reader.read();
//     curr_char = file_contents.begin();
//     end = file_contents.end();
//     this->filename = filename;
//     path = std::filesystem::current_path() / filename;
//     return true;
// }

masm::lexer::Lexer::Lexer(prep::Prep *p)
{
    _p = p;
    file_contents = p->get_final_contents();
    curr_char = file_contents.begin();
    end = file_contents.end();
}

void masm::lexer::Lexer::setup(prep::Prep *p)
{
    _p = p;
    file_contents = p->get_final_contents();
    curr_char = file_contents.begin();
    end = file_contents.end();
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
    if (should_skip((*curr_char)))
    {
        clear_unnecessary();
    }
    if (*curr_char == ';' && peek() == ';')
    {
        // this is a comment and we can completely skip this
        while (*curr_char == ';' && peek() == ';')
            consume_comment();
    }
    if (curr_char == end)
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
    else if (is_alpha(*curr_char) || *curr_char == '.' || *curr_char == '_')
    {
        if (*curr_char == '.')
        {
            consume();
        }
        token = get_iden_or_keyword();
    }
    else if (*curr_char == '-')
    {
        consume();
        token = get_number();
        token.value.insert(token.value.begin(), '-');
        token.type = (TokenType)((size_t)token.type + 1);
    }
    else if (is_num(*curr_char))
    {
        token = get_number();
    }
    else if (*curr_char == '\"' || *curr_char == '\'')
    {
        // get the string
        token.type = _TT_STRING;
        token.value = get_string();
    }
    else
    {
        std::string _;
        _ += *curr_char;
        invalid_token(_);
    }
    return token;
}

void masm::lexer::Lexer::invalid_token(std::string read)
{
    auto details = _p->get_file(line_num);
    error::_print_err_(details._file_name_, line_num - details.start, col_no - read.length(), _CONTEXT_LEX_, "Invalid token received.", get_current_line());
}

void masm::lexer::Lexer::lex_error(std::string msg)
{
    auto details = _p->get_file(line_num);
    error::_print_err_(details._file_name_, line_num - details.start, 0, _CONTEXT_LEX_, msg, get_current_line());
}

void masm::lexer::Lexer::parse_err_whole_line(std::string msg)
{
    auto details = _p->get_file(line_num);
    error::_print_err_(details._file_name_, line_num - details.start, 0, _CONTEXT_PARSE_, msg, get_current_line());
}

void masm::lexer::Lexer::parse_err_expected_colon(std::string msg)
{
    auto details = _p->get_file(line_num);
    std::cerr << "While " << _CCODE_BOLDWHITE << "Parsing:\n";
    std::cerr << details._file_name_ << ":" << line_num << ":" << _CCODE_RESET;
    std::cerr << " "
              << "Expected ':'" << msg << std::endl;
    std::cerr << "Aborting further compilation." << std::endl;
    exit(EXIT_FAILURE);
}

void masm::lexer::Lexer::parse_err_previous_token(std::string prev_tok, std::string msg)
{
    auto details = _p->get_file(line_num);
    std::cerr << "While " << _CCODE_BOLDWHITE << "Parsing:\n";
    std::cerr << details._file_name_ << ":" << line_num << ":" << _CCODE_RESET;
    std::cerr << " " << msg << std::endl;
    std::cerr << "Aborting further compilation." << std::endl;
    exit(EXIT_FAILURE);
}

void masm::lexer::Lexer::parse_error(std::string msg)
{
    auto details = _p->get_file(line_num);
    std::cerr << "While " << _CCODE_BOLDWHITE << "Parsing:\n";
    std::cerr << details._file_name_ << ":" << line_num << ":" << _CCODE_RESET;
    std::cerr << " " << msg << std::endl;
    std::cerr << "Aborting further compilation." << std::endl;
    exit(EXIT_FAILURE);
}

// std::vector<masm::lexer::Token> masm::lexer::Lexer::lex_all()
// {
//     std::vector<Token> alltoks;
//     Token token;
//     std::string val;
//     while (true)
//     {
//         if (std::isspace(*curr_char) || *curr_char == '\n')
//         {
//             clear_unnecessary();
//         }
//         if (*curr_char == '/' && peek() == '/')
//         {
//             // this is a comment and we can completely skip this
//             while (*curr_char == '/' && peek() == '/')
//                 consume_comment();
//         }
//         if (curr_char == file_contents.end())
//         {
//             alltoks.push_back(Token(_TT_EOF, ""));
//             break;
//         }
//         if (is_oper(*curr_char))
//         {
//             val.push_back(*curr_char);
//             consume();
//             // we don't have multi-character operators and so we can skip it as well
//             auto operkind = _iden_map_.find(val);
//             // operkind should be valid
//             token.type = (*operkind).second;
//         }
//         else if (is_alpha(*curr_char))
//         {
//             token = get_iden_or_keyword();
//         }
//         else if (is_num(*curr_char))
//         {
//             token = get_number();
//         }
//         else
//         {
//             invalid_token();
//         }
//         alltoks.push_back(token);
//         val.clear();
//     }
//     return alltoks;
// }