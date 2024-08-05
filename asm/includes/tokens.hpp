#ifndef _TOKENS_
#define _TOKENS_

#include <string>
#include <unordered_map>

namespace masm
{
    enum TokenType
    {
        TOK_ERR,
        TOK_EOF,
        TOK_NUM,
        TOK_FLOAT,
        TOK_ID,
    };

    struct Location
    {
        size_t line;
        size_t col_st;
        size_t offset;
    };

    struct Token
    {
        TokenType type;
        std::string val;
        Location loc;
    };

    static std::unordered_map<std::string, TokenType> keymap;
};

#endif