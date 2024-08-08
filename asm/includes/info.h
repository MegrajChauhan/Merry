#ifndef _INFO_
#define _INFO_

#include "merry_types.h"
#include "defs.h"

typedef struct UnitInfo UnitInfo;
typedef struct Context Context;
typedef struct CompUnit CompUnit;

struct UnitInfo
{
    msize_t current_line;
    msize_t current_offset;
    msize_t current_column;
    mstr_t inp_file_path;
    mstr_t inp_file_contents;
    msize_t inp_file_len; // We will have the '\0' obviously
};

struct CompUnit
{
    UnitInfo info;
    mbool_t included; // Indicating if this is an included file
    mbool_t f_read;   // indicating if the file has been read
    CompUnit *parent; // Which unit included it?
    State state;
};

struct Context
{
    CompUnit **units;
    msize_t unit_count;
};

Context *generate_context();

mret_t add_comp_unit(Context *c, mstr_t path);

void destroy_context(Context *c);

#endif