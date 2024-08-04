#ifndef _LEXER_
#define _LEXER_

#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <optional>
#include "error.hpp"
#include "keywords.hpp"

// We lex line by line

#define iscomment(iter) ((*iter != '\0') && (*iter == ';') && (peek() == ';'))

namespace masm
{
    struct Pos
    {
        size_t line;
        size_t col_st;
        size_t col_end;
        std::shared_ptr<std::string> _file = nullptr;
    };

    struct Token
    {
        TokenType type;
        std::string val;
        Pos p;
    };

    class Lexer
    {
        size_t line = 0, col = 0;

    public:
        Lexer() = default;

        std::optional<std::vector<Token>> lex_line(std::shared_ptr<std::string> file, std::shared_ptr<std::string> line, size_t lno);

    };
};

#endif