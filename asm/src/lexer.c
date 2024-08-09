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

mret_t next_token(CompUnit *unit, Token *t)
{
    Token t;
    while (!eof(unit))
    {
        if (isspace(unit->curr_char))
        {
            while (!eof(unit) && isspace(unit->curr_char))
                consume(unit);
        }
        else if (isdigit(unit->curr_char))
        {
            t->val = unit->curr_char;
            int dot_count = 0;
            while (isdigit(unit->curr_char) || unit->curr_char == '.')
            {
                if (unit->curr_char == '.')
                    dot_count++;
                if (dot_count > 1)
                {
                    
                }
            }
        }
    }
}