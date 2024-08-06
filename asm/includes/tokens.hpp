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
        TOK_BARRAY, // array of bytes
        TOK_FLOAT,
        TOK_ID,
        TOK_USE,
        TOK_SEMI,   // :
        TOK_BYTE,   // byte
        TOK_WORD,   // word
        TOK_DWORD,  // dword
        TOK_QWORD,  // qword
        TOK_RBYTE,  // rbyte
        TOK_RWORD,  // rword
        TOK_RDWORD, // rdword
        TOK_RQWORD, // rqword
        TOK_FLOAT,  // float
        TOK_LFLOAT, // lfloat
        TOK_STRING, // string
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

    static std::unordered_map<std::string, TokenType>
        keymap =
            {
                {"use", TOK_USE}, {"byte", TOK_BYTE}, {"word", TOK_WORD}, {"dword", TOK_DWORD}, {"qword", TOK_QWORD}, {"rbyte", TOK_RBYTE}, {"rword", TOK_RWORD}, {"rdword", TOK_RDWORD}, {"rqword", TOK_RQWORD}, {"string", TOK_STRING}, {"float", TOK_FLOAT}, {"lfloat", TOK_LFLOAT}};
};

#endif