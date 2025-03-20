#include "merry_dynamic_libs.h"

mdlentry_t merry_load_dynamic_library(mstr_t path)
{
    merry_check_ptr(path);
    mdlentry_t entry;
#if defined(_USE_LINUX_)
    entry = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (entry == NULL)
        return RET_NULL;
#elif defined(_USE_WIN_)
    entry = LoadLibrary(TEXT(lib_path));
    if (entry == NULL)
        return RET_NULL;
#endif
    return entry;
}

void merry_unload_dynamic_library(mdlentry_t entry)
{
    merry_check_ptr(entry);
#if defined(_USE_LINUX_)
    dlclose(entry);
#elif defined(_USE_WIN_)
    FreeLibrary(entry);
#endif
}

mptr_t merry_get_symbol_from_dynamic_library(mdlentry_t entry, mstr_t sym_name)
{
    merry_check_ptr(entry);
    merry_check_ptr(sym_name);
#if defined(_USE_LINUX_)
    return dlsym(entry, sym_name);
#elif defined(_USE_WIN_)
    return GetProcAddress(entry, sym_name);
#endif
}
