#include "log.h"

void log(CompUnit *unit, mstr_t kind, mstr_t msg)
{
    char *st, *ed;
    st = ed = unit->curr_char;
    while (st != unit->info.inp_file_contents && st != '\n')
        st--;
    st++;
    while (ed != '\0' && ed != '\n')
        ed++;
    ed--;

    char tmp[10];
    switch (unit->state)
    {
    case LEXING:
        strcpy(tmp, "Lexing\0");
        break;
    case PARSING:
        strcpy(tmp, "Parsing\0");
        break;
    }

    msize_t len = ed - st;

    // logging
    // LOGS TO STDERR BY DEFAULT
    fprintf(stderr, "%s:\n\tWhile %s in line %lu starting from column %lu:[%s]\n\t%s\n\n\t", unit->info.inp_file_path, tmp, unit->info.current_line, unit->info.current_column, kind, msg);
    for (msize_t i = 0; i < len; i++)
    {
        putc(st, stderr);
        st++;
    }
    putc('\n', stderr);
    putc('\t', stderr);
    for (msize_t i = 0; i < unit->info.current_column; i++)
        putc(' ', stderr);
    fprintf(stderr, "^-- Starting from here.\n");
}