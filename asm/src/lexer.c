#include "lexer.h"

void consume(CompUnit *unit)
{
    if (unit->eof == mtrue)
        return;
    if (unit->info.inp_file_len == unit->info.current_offset)
    {
        unit->eof = mtrue;
        return;
    }
    if (unit->curr_char == '\n')
    {
        unit->info.current_column = 0;
        unit->info.current_line++;
    }
    else
        unit->info.current_column++;
    unit->curr_char++;
    unit->info.current_offset++;
}

char peek(CompUnit *unit, msize_t peek_by)
{
    if (unit->eof == mtrue || unit->info.current_offset == unit->info.inp_file_len || (unit->info.current_offset + peek_by) >= unit->info.inp_file_len)
        return '\0';
    return *(unit->curr_char + peek_by);
}

mret_t next_token(CompUnit *unit, Token *t)
{
    while (!eof(unit))
    {
        if (isspace(*(unit->curr_char)))
        {
            while (!eof(unit) && isspace(*(unit->curr_char)))
                consume(unit);
        }
        else if (isdigit(*(unit->curr_char)))
        {
            t->val = unit->curr_char;
            t->loc.col_st = unit->info.current_column;
            t->loc.line = unit->info.current_line;
            t->loc.offset = unit->info.current_offset;
            int dot_count = 0;
            while (isdigit(*(unit->curr_char)) || *(unit->curr_char) == '.')
            {
                if (*(unit->curr_char) == '.')
                    dot_count++;
                if (dot_count > 1)
                {
                    log(unit, FATAL, "A second '.' was found which is not allowed in a floating point number.");
                    t->type = TOK_ERR;
                    return RET_FAILURE;
                }
                consume(unit);
            }
            if (dot_count == 0)
                t->type = TOK_NUM;
            else
                t->type = TOK_FLOAT;
            t->len = (unit->curr_char - t->val);
            break;
        }
        else if (*unit->curr_char == ';' && peek(unit, 1) == ';')
        {
            while (*unit->curr_char != '\n' && !eof(unit))
                consume(unit);
        }
        else if (isalpha(*(unit->curr_char)) || *(unit->curr_char) == '_')
        {
            t->val = unit->curr_char;
            t->loc.col_st = unit->info.current_column;
            t->loc.line = unit->info.current_line;
            t->loc.offset = unit->info.current_offset;
            while (isalpha(*(unit->curr_char)) || *(unit->curr_char) == '_')
                consume(unit);
            t->len = unit->curr_char - unit->info.inp_file_contents;
            
            break;
        }
    }
    return RET_SUCCESS;
}