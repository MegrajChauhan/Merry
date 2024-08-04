#ifndef _KEYS_
#define _KEYS_

#include <unordered_map>
#include <string>

namespace masm
{
    enum TokenType
    {
        TOK_ID,
        TOK_COMMENT,
        TOK_NUM,
        TOK_FLOAT,
    };

    static std::unordered_map<std::string, TokenType> keys;
};

#endif