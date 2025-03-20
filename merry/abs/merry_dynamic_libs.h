#ifndef _MERRY_DYNL_
#define _MERRY_DYNL_

#include <merry_config.h>
#include <merry_utils.h>
#include <merry_types.h>
#include <merry_platform.h>

// we don't know what type of function we might want

mdlentry_t merry_load_dynamic_library(mstr_t path);

void merry_unload_dynamic_library(mdlentry_t entry);

// The calling component shall, by themselves, make the correct type cast
mptr_t merry_get_symbol_from_dynamic_library(mdlentry_t entry, mstr_t sym_name);

// This should do for now but we may need to modify this when we need
// special types of libraries with proper flags

#endif