#ifndef _LEXER_
#define _LEXER_

#include <stdlib.h>
#include "reader.h"

typedef struct Lexer
{
    Reader *reader;
    size_t curr_col;
    size_t curr_line;
    int eof; // is eof?
} Lexer;

// token type
enum
{
    _TT_EOF,
};

typedef size_t token_t;

typedef struct Token
{
    token_t type;
    char *val;
} Token;

Lexer *init_lexer(Reader *reader);

void destory_lexer(Lexer *lexer);

// utility functions
char peek(Lexer *lexer);

void consume(Lexer *lexer);

#endif