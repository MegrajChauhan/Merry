#ifndef _COMPUNIT_
#define _COMPUNIT_

#include <stdio.h>
#include "symtable.h"

typedef struct compunit compunit;

struct compunit
{
    symtable *table;
};

#endif