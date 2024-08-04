#include "lexer.hpp"

std::optional<std::vector<masm::Token>> masm::Lexer::lex_line(std::shared_ptr<std::string> file, std::shared_ptr<std::string> line, size_t lno)
{
    std::string::iterator i = line.get()->begin();
    std::string strline = *line.get();
    std::string filename = *file.get();
    std::vector<masm::Token> toks;
    col = 0;
    while (*i != '\0')
    {
        Token t;
        size_t st = col;
        if (*i == ';')
        {
            if (*(i + 1) != ';')
            {
                mlog(filename, lno, st, col, FATAL, "Comments start with ';;' but give just ';'.", strline);
                return {}; //  we cannot recover at this point
            }
            else
                return std::make_optional<std::vector<Token>>({Token{.type = TOK_COMMENT}});
        }
        else if (std::isspace(*i))
        {
            while (std::isspace(*i))
            {
                i++;
                col++;
            }
        }
        else if (std::isdigit(*i))
        {
            int dot_count = 0;
            while (std::isdigit(*i) || *i == '.')
            {
                if (*i == '.')
                    dot_count++;
                if (dot_count > 1)
                {
                    mlog(filename, lno, st, col, FATAL, "Didn't expect more than two '.' for a floating point number.", strline);
                    return {}; // an unrecoverable error
                }
                t.val += *i;
                col++;
                i++;
            }
            if (dot_count == 0)
                t.type = TOK_NUM;
            else
                t.type = TOK_FLOAT;
        }
        else if (std::isalpha(*i))
        {
            while (std::isalnum(*i) || *i == '_')
            {
                t.val += *i;
                i++;
                col++;
            }
            auto key = keys.find(t.val);
            if (key == keys.end())
                // not a keyword
                t.type = TOK_ID;
            else
                t.type = key->second;
        }
        t.p = Pos{lno, st, col, file};
        toks.push_back(t);
    }
    return std::make_optional<std::vector<Token>>(toks);
}

// void masm::Lexer::consume()
// {
//     if (*iter == '\0')
//         return;
//     offset++;
//     iter++;
//     if (*iter == '\n')
//     {
//         col = 0;
//         line++;
//         offset++;
//         iter++;
//         return;
//     }
//     col++;
// }

// char masm::Lexer::peek()
// {
//     return *iter == '\0' ? '\0' : *(iter + 1);
// }

// std::string masm::Lexer::get_current_token()
// {
//     std::string tok;
//     while ((std::isalnum(*iter) || *iter == '_') && peek() != '\n')
//     {
//         tok += *iter;
//         consume();
//     }
//     return tok;
// }

// void masm::Lexer::consume_comments()
// {
//     while (peek() != '\n')
//         consume();
//     consume();
//     // get rid of any spaces as well
//     if (std::isspace(*iter))
//         while (std::isspace(*iter))
//             consume();
// }

// masm::Token masm::Lexer::next_token()
// {
//     Token t;
//     if (std::isspace(*iter))
//         while (std::isspace(*iter))
//             consume();
//     if (iscomment(iter))
//         while (iscomment(iter))
//             consume_comments();
//     if (std::isalpha(*iter))
//     {
//         // could be an identifier, keyword or something
//         std::string curr_tok = get_current_token();
//         auto x = keys.find(curr_tok);
//         if (x != keys.end())
//         {

//         }
//     }
// }