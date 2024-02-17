#ifndef _MERRY_DYNL_
#define _MERRY_DYNL_

#include "../utils/merry_types.h"
#include <stdlib.h>
#include <string.h>

#if defined(_MERRY_HOST_OS_LINUX_)
#define _USE_LINUX_
#include <dlfcn.h> // for loading a library dynamically
#endif

typedef struct MerryDynEntry MerryDynEntry;
typedef struct MerryDynLoader MerryDynLoader;

_MERRY_DEFINE_FUNC_PTR_(mdword_t, dynfunc_t, mptr_t ptr)

struct MerryDynEntry
{
    char entry_name[255]; // the library's name or path
    mptr_t lib_handle;    // the loaded library's handle
    mbool_t handle_open;  // is the library open to use?
};

struct MerryDynLoader
{
    MerryDynEntry *entries; // the entries
    msize_t entry_count;    // number of entries
    msize_t closed_entry_count;
};

static MerryDynLoader loader;

mbool_t merry_loader_init(msize_t initial_entry_count);

void merry_loader_close();

mbool_t merry_loader_loadLib(mstr_t lib_path, msize_t *handle);

void merry_loader_unloadLib(msize_t handle);

// each function in the library must return unsigned int which will be the return value
// the return value of the function is loaded into the Ma register
dynfunc_t merry_loader_getFuncSymbol(msize_t handle, mstr_t sym_name);

#endif