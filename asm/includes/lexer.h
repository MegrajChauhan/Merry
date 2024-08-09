#ifndef _LEXER_
#define _LEXER_

#include "info.h"

typedef enum TokenType TokenType;
typedef struct Token Token;
typedef struct Location Location;

enum TokenType
{
    TOK_EOF,
    TOK_ERR,
    TOK_NUM,
};

struct Location
{
    msize_t line;
    msize_t col_st;
    msize_t offset;
};

struct Token
{
    TokenType type;
    Location loc;
    mstr_t val; // This will just be the pointer
    msize_t len;
};

// We need to work with individual tokens
// Once the parser makes sense of a few tokens, it will create the nodes which will be just a linked list
// std::vector would be nice but we are using pure C so of course.

#define eof(unit) (unit->eof == mtrue)
#define isdigit(c) ((c >= '0' && c <= '9'))
#define islower(c) (c >= 'a' && c <= 'z')
#define isupper(c) (c >= 'A' && c <= 'Z')
#define isalpha(c) (isupper(c) || islower(c))
#define isalnum(c) (isdigit(c) || isalpha(c))
#define isspace(c) (c == ' ' || c == '\r' || c == '\t' || c == '\n' || c == "\f" || c == '\v')

mret_t next_token(CompUnit *unit, Token *t);

void consume(CompUnit *unit);

#endif