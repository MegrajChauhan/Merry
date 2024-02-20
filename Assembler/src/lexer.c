#include "../includes/lexer.h"

Lexer *init_lexer(Reader *reader)
{
    if (reader == NULL)
        return NULL;
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    if (lexer == NULL)
        return NULL;
    lexer->reader = reader;
    lexer->curr_col = 0;
    lexer->curr_line = 0;
    lexer->eof = 0;
    return lexer;
}

void destroy_lexer(Lexer *lexer)
{
    if (lexer == NULL)
        return;
    // the reader is freed by the lexer
    destroy_reader(lexer->reader);
    free(lexer);
}

char peek(Lexer *lexer)
{
    if (_ITER_SAME_(lexer->reader->iter))
    {
        // we are at EOF
        lexer->eof = 1;
        return '\0';
    }
    return *((char *)(lexer->reader->iter.start) + 1);
}

void consume(Lexer *lexer)
{
    if (_ITER_SAME_(lexer->reader->iter))
    {
        // reached EOF
        lexer->eof = 1;
        return;
    }
    if (*(char *)(lexer->reader->iter.start) == '\n')
    {
        lexer->curr_line++;
        lexer->curr_col = 0;
    }
    else
    {
        lexer->curr_col++;
    }
    lexer->reader->iter.start++;
}
