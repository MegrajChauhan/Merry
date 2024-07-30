#ifndef _SYMTABLE_
#define _SYMTABLE_

#include <stdlib.h>
#include "hash.h"

typedef struct symtable symtable;
typedef struct symtable_entry symtable_entry;

enum
{
    _NONE,
};

struct symtable_entry
{
    char *_entry_name;
    int _entry_type;
    char *_entry_val;
    symtable_entry *next_entry;
};

struct symtable
{
    hashmap *table;
    
};

#endif