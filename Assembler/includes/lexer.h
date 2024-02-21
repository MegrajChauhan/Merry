#ifndef _LEXER_
#define _LEXER_

#include <stdlib.h>
#include "reader.h"

#define isalpha(ch) ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
#define isnum(ch) (ch >= '0' && ch <= '9')
#define isunnecessary(ch) (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t')

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
    _TT_ERR, // return for functions
    _TT_EOF,
    _TT_SECTION_DATA,
    _TT_SECTION_TEXT,
    _TT_KEY_STRING, // the ".string" keyword
    _TT_KEY_DB, // "db" or define byte
    _TT_KEY_PROC,

    _TT_TYPE_STRING,
    _TT_TYPE_INT,

    _TT_OPER_COLON,

    _TT_IDENTIFIER,

    // for the lexer, even the register names like Ma, Mb are IDENTIFIER, it's interpretation is upto the parser 
    _TT_INST_MOV,
    _TT_INST_HLT,
    
    // we ignore commas, they are not absolutely necessary and the assembler won't even complain
    // about not using it. It is just their to provide readability
};

typedef size_t token_t;

typedef struct Token
{
    token_t type;
    char *val;
} Token;

Lexer *init_lexer(Reader *reader);

void destory_lexer(Lexer *lexer);

Token get_next_token(Lexer *lexer);

// utility functions
char peek(Lexer *lexer);

void consume(Lexer *lexer);

void consume_whitespace(Lexer *lexer);

char current_char(Lexer *lexer);

int is_comment(Lexer *lexer);

void consume_unnecessary(Lexer *lexer);

void consume_comments(Lexer *lexer);

token_t get_word(Lexer *lexer);

token_t identify(char *word);

#endif