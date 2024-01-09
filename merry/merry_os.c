#include "internals/merry_os.h"

Merry *merry_os_init(mcstr_t _inp_file)
{
    // initialize the os
    Merry *os = (Merry *)merry_malloc(sizeof(Merry));
    if (os == RET_NULL)
        return RET_NULL; // we failed
    // just 1 core
    // os->cores = (Merry **)merry_malloc(sizeof(Merry *));
    if (os->cores == RET_NULL)
    {
        merry_free(os);
        return RET_NULL;
    }
    // we need memory to be initialized
    os->core_count = 0; // we will start with one core
}

mret_t merry_os_mem_read_data(Merry *os, maddress_t address, mqptr_t store_in, msize_t core_id)
{
    // we can use different read/write based on the situation
    // os will surely not be NULL
    // store_in is either a reference or pointer to the core's register or something
    // core_id is the unique value provided to cores by the OS which is used for identification
    // since this request was made then the core is not NULL either
    if (os->cores[core_id]->_is_private == mtrue)
    {
        // this core is assuring that it's data is not accessed by other cores
        return merry_memory_read(os->data_mem, address, store_in);
    }
    else
    {
        return merry_manager_mem_read_data(os, address, store_in);
    }
    return RET_FAILURE;
}

mret_t merry_os_mem_write_data(Merry *os, maddress_t address, mqword_t to_store, msize_t core_id)
{
    if (os->cores[core_id]->_is_private == mtrue)
    {
        // this core is assuring that it's data is not accessed by other cores
        return merry_memory_write(os->data_mem, address, to_store);
    }
    else
    {
        return merry_manager_mem_write_data(os, address, to_store);
    }
    return RET_FAILURE;
}