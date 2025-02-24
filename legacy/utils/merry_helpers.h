#ifndef _HELPERS_
#define _HELPERS_

#include "merry_types.h"
#include <string.h>

typedef union MerryPtoQ MerryPtoQ;

union MerryPtoQ
{
    mptr_t ptr;
    mqword_t qword;
};

mbool_t str_starts_with(mstr_t src, mstr_t _prefix);

mbool_t str_ends_with(mstr_t src, mstr_t _suffix);

#endif