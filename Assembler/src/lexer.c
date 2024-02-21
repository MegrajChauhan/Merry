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

void consume_whitespace(Lexer *lexer)
{
    while (lexer->eof == 0 && *(char *)lexer->reader->iter.start == ' ')
    {
        consume(lexer);
    }
}

char current_char(Lexer *lexer)
{
    return *(char *)lexer->reader->iter.start;
}

void consume_unnecessary(Lexer *lexer)
{
    while (lexer->eof == 0 && isunnecessary(current_char(lexer)))
    {
        consume(lexer);
    }
}

int is_comment(Lexer *lexer)
{
    return current_char(lexer) == ';' && peek(lexer) == ';';
}

void consume_comments(Lexer *lexer)
{
    // consuming comments is just reading until we hit a new line
    while (1)
    {
        if (current_char(lexer) == ';' && peek(lexer) == ';')
        {
            // this is a comment
            while (lexer->eof == 0 && current_char(lexer) != '\n')
            {
                consume(lexer);
            }
            consume_unnecessary(lexer);
        }
        else
        {
            break;
        }
    }
}

Token get_next_token(Lexer *lexer)
{
    token_t type;
    char *val;
    Token tok;

    char current = current_char(lexer);

    if (isunnecessary(current))
    {
        consume_unnecessary(lexer);
    }
    if (is_comment(lexer))
    {
        consume_comments(lexer);
    }
    if (lexer->eof == 1)
    {
        tok.type = _TT_EOF;
        return tok;
    }
    // check for segments
    current = current_char(lexer);
    if (current == '.')
    {
        // must be a segemnt since floating point numbers can't have '.' at the start
    }

    tok.type = type;
    tok.val = val;
    return tok;
}

token_t get_word(Lexer *lexer)
{
    // get a group of text
    char *temp = (char *)(lexer->reader->iter.start);
    while (lexer->eof == 0 && isalpha(current_char(lexer)))
    {
        consume(lexer);
    }
    // now we need to get the size
    size_t len = lexer->reader->iter.start - (void *)temp;
    char tok[len];
    // now extract the word
    strncpy(tok, lexer->reader->iter.start - len, len);
    // now identify the token
    return identify(tok);
}

token_t identify(char *word)
{
    switch (*word)
    {
    case 't':
    {
        // if it is 't' then it could be 'text'
        if (strcmp(word, "text") == 0)
            return _TT_SECTION_TEXT;
        break;
    }
    case 'd':
    {
        // could be data
        if (strcmp(word, "data") == 0)
            return _TT_SECTION_DATA;
        if (strcmp(word, "db") == 0)
            return _TT_KEY_DB;
        break;
    }
    }
}