#include "info.h"

Context *generate_context()
{
    Context *c = (Context *)malloc(sizeof(Context));
    if (c == NULL)
        return NULL;
    c->unit_count = 0;
    c->units = NULL;
    return c;
}

void destroy_compunit(CompUnit *unit)
{
    if (unit == NULL)
        return;
    if (unit->f_read == mtrue && unit->info.inp_file_contents != NULL)
        free(unit->info.inp_file_contents);
    if (unit->info.inp_file_path != NULL)
        free(unit->info.inp_file_path);
    if (unit->included == mtrue)
        unit->parent = NULL;
}

void destroy_context(Context *c)
{
    if (c == NULL)
        return;
    if (c->unit_count == 0)
        return;
    for (msize_t i = 0; i < c->unit_count; i++)
    {
        destroy_compunit(&c->units[i]);
    }
    free(c->units);
    free(c);
}

mret_t add_comp_unit(Context *c, mstr_t path, mbool_t is_inc, CompUnit *parent)
{
    if (c->unit_count == 0)
        c->units = (CompUnit *)malloc(sizeof(CompUnit));
    else
        c->units = (CompUnit *)realloc(c->units, sizeof(CompUnit) * (c->unit_count + 1));
    if (c->units == NULL)
        return RET_FAILURE;
    // Here, the failure to allocate for one unit indicates that we cannot continue the compilation
    // Thus, we have to terminate the assembler and then exit.
    c->units[c->unit_count].f_read = mfalse;
    c->units[c->unit_count].included = is_inc;
    c->units[c->unit_count].parent = parent;
    c->units[c->unit_count].state = NONE;
    c->units[c->unit_count].info.inp_file_path = path;
    c->units[c->unit_count].curr_char = NULL;
    c->units[c->unit_count].eof = mfalse;
    c->unit_count++;
    return RET_SUCCESS;
}