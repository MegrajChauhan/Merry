#include "merry_dynl.h"

mbool_t merry_loader_init(msize_t initial_entry_count)
{
    inlog("COMPONENT INIT: Loader initializing");
    loader.entries = (MerryDynEntry *)malloc(sizeof(MerryDynEntry) * initial_entry_count);
    if (loader.entries == NULL)
    {
        mreport("COMPONENT INIT: Loader initialization Failed");
        return mfalse;
    }
    loader.closed_entry_count = initial_entry_count;
    loader.entry_count = initial_entry_count;
    for (msize_t i = 0; i < initial_entry_count; i++)
    {
        loader.entries[i].handle_open = mtrue;
    }
    inlog("COMPONENT INIT: Loader initialization Succeeded");
    return mtrue;
}

void merry_loader_close()
{
    // all the open library handles must be closed
    // but for safety purposes, we will check
    inlog("COMPONENT DEINIT: Loader de-initializing");
    if (loader.closed_entry_count != loader.entry_count)
    {
        for (msize_t i = 0; i < loader.entry_count; i++)
        {
            if (loader.entries[i].handle_open == mfalse)
            {
#if defined(_USE_LINUX_)
                dlclose(loader.entries[i].lib_handle);
#elif defined(_USE_WIN_)
                FreeLibrary(loader.entries[i].lib_handle);
#endif
            }
        }
    }
    if (loader.entries != NULL)
        free(loader.entries);
}

_MERRY_INTERNAL_ msize_t merry_loader_find_free_handle()
{
    for (msize_t i = 0; i < loader.entry_count; i++)
    {
        if (loader.entries[i].handle_open == mtrue)
            return i;
    }
    // this is not reachable
    return loader.entry_count + 1;
}

_MERRY_INTERNAL_ mbool_t merry_loader_add_entry()
{
    // if adding this doesn't work than the manager will exit the VM
    // since the addition of the library must be important for furthur execution
    loader.entries = (MerryDynEntry *)realloc(loader.entries, loader.entry_count + 1);
    if (loader.entries == NULL)
        return mfalse;
    loader.entry_count++;
    loader.closed_entry_count++;
    return mtrue;
}

_MERRY_INTERNAL_ msize_t merry_loader_check_entry(mstr_t path)
{
    for (msize_t i = 0; i < loader.entry_count; i++)
    {
        if (strcmp(loader.entries[i].entry_name, path) == 0)
            return i;
    }
    return loader.entry_count + 1;
}

mbool_t merry_loader_loadLib(mstr_t lib_path, msize_t *handle)
{
    log("Loading library %s%s%s", BOLDWHITE, lib_path, RESET);
    if ((*handle = merry_loader_check_entry(lib_path)) < loader.entry_count)
        goto _done;
    if (loader.closed_entry_count > 0)
    {
        // we have a free handle some where
        *handle = merry_loader_find_free_handle();
    }
    else
    {
        // we need to add one more entry
        if (merry_loader_add_entry() == mfalse)
        {
            inlog("Failed to add a new LOADER entry");
            goto _failed;
        }
        *handle = loader.entry_count - 1;
    }
// now we load the library
#if defined(_USE_LINUX_)
    loader.entries[*handle].lib_handle = dlopen(lib_path, RTLD_NOW | RTLD_GLOBAL);
    if (loader.entries[*handle].lib_handle == NULL)
    {
        inlog("Failed to open the library(SYS ERR)");
        goto _failed;
    }
#elif defined(_USE_WIN_)
    loader.entries[*handle].lib_handle = LoadLibrary(TEXT(lib_path));
    if (loader.entries[*handle].lib_handle == NULL)
    {
        inlog("Failed to open the library(SYS ERR)");
        goto _failed;
    }
#endif
    loader.entries[*handle].handle_open = mfalse; // this handle is closed now
    loader.closed_entry_count--;
    strcpy(loader.entries[*handle].entry_name, lib_path);
_done:
    log("Loaded library %s%s%s", BOLDWHITE, lib_path, RESET);
    return mtrue; // the library is loaded
_failed:
    log("Failed to Load library %s%s%s", BOLDWHITE, lib_path, RESET);
    return mfalse;
}

mptr_t merry_loadLib(mstr_t libname)
{
    log("Loading SUBSYSTEM %s%s%s", BOLDWHITE, libname, RESET);
#if defined(_USE_LINUX_)
    mptr_t handle = dlopen(libname, RTLD_NOW | RTLD_GLOBAL);
    if (handle == NULL)
    {
        log("Loading SUBSYSTEM(%s%s%s); Process failed(SYS ERR).", BOLDWHITE, libname, RESET);
        return RET_NULL;
    }
#elif defined(_USE_WIN_)
    handle = LoadLibrary(TEXT(lib_path));
    if (handle == NULL)
    {
        log("Loading SUBSYSTEM(%s%s%s); Process failed(SYS ERR).", BOLDWHITE, libname, RESET);
        return RET_NULL;
    }
#endif
    return handle;
}

void merry_loader_unloadLib(msize_t handle)
{
    // this will not throw any error
    // while it should but not now
    if (handle >= loader.entry_count)
        return;
    if (loader.entries[handle].handle_open == mtrue)
        return; // it is already closed
#if defined(_USE_LINUX_)
    dlclose(loader.entries[handle].lib_handle);
#elif defined(_USE_WIN_)
    FreeLibrary(loader.entries[handle].lib_handle);
#endif
    loader.entries[handle].handle_open = mtrue;
    loader.closed_entry_count++;
}

void merry_unloadLib(mptr_t *handle)
{
#if defined(_USE_LINUX_)
    dlclose(*handle);
#elif defined(_USE_WIN_)
    FreeLibrary(*handle);
#endif
}

dynfunc_t merry_loader_getFuncSymbol(msize_t handle, mstr_t sym_name)
{
    if (handle >= loader.entry_count)
        return RET_NULL;
    if (loader.entries[handle].handle_open == mtrue)
        return RET_NULL; // it is already closed
#if defined(_USE_LINUX_)
    return (dynfunc_t)dlsym(loader.entries[handle].lib_handle, sym_name);
#elif defined(_USE_WIN_)
    return (dynfunc_t)GetProcAddress(loader.entries[handle].lib_handle, sym_name);
#endif
    return RET_NULL;
}

subsys_t merry_libsym(mptr_t handle, mstr_t symname)
{
#if defined(_USE_LINUX_)
    return (subsys_t)dlsym(handle, symname);
#elif defined(_USE_WIN_)
    return (subsys_t)GetProcAddress(handle, symname);
#endif
    return RET_NULL;
}

mbool_t merry_loader_is_still_valid(msize_t handle)
{
    if (handle >= loader.entry_count)
        return mfalse;
    return loader.entries[handle].handle_open;
}

// The reason we aren't adding log statements to every function is because there is no reason
// nor merit in doing that. If we have proper initialization and proper args, the rest of the
// functions won't fail