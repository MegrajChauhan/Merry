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
    MerryInpFile *input = merry_read_file(_inp_file);
    if (input == RET_NULL)
    {
        // the input file was not read and we failed
        merry_free(os);
        return RET_NULL;
    }
    // dlen and ilen will provide us with the number of bytes
    // convert that to number of pages
    msize_t dpage_len = input->dlen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_ + (input->dlen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_ > 0 ? 1 : 0); // get the number of pages
    if (dpage_len == 0)
        dpage_len = 1;                                                                                                                   // we start with 1 page if nothing else
    msize_t ipage_len = input->ilen / _MERRY_MEMORY_ADDRESSES_PER_PAGE_ + (input->ilen % _MERRY_MEMORY_ADDRESSES_PER_PAGE_ > 0 ? 1 : 0); // get the number of pages
    // initialize the memory
    if ((os->data_mem = merry_memory_init_provided(input->_data, input->dpage_count)) == RET_NULL)
    {
        // grand failure
        merry_free(os);
        merry_destory_reader(input);
        return RET_NULL;
    }
    // perform initialization for the inst mem as well. Unlike data memory, instruction page len cannot be 0
    // based on the size of the input file, the reader can independently run in the background as it reads the input file
    // the reader doesn't concern itself with the OS and so it can run independently
    // all it has to do is map the necessary pages and return us a pointer
    // This way it can continue to read in the background and the VM can also continue without any problems
    if ((os->inst_mem = merry_memory_init_provided(input->_instructions, input->ipage_count)) == RET_NULL)
    {
        // grand failure
        merry_memory_free(os->data_mem);
        merry_destory_reader(input);
        merry_free(os);
        return RET_NULL;
    }
    // time for locks and mutexes
    if ((os->_cond = merry_cond_init()) == RET_NULL)
    {
        merry_os_destroy(os);
        merry_destory_reader(input);
        return RET_NULL;
    }
    if ((os->_lock = merry_mutex_init()) == RET_NULL)
    {
        merry_os_destroy(os);
        merry_destory_reader(input);
        return RET_NULL;
    }
    // don't forget to destory the reader
    merry_destory_reader(input);
    os->core_count = 0; // we will start with one core
    os->cores = (MerryCore **)merry_malloc(sizeof(MerryCore *));
    if (os->cores == RET_NULL)
        goto failure;
    os->cores[0] = merry_core_init(os->inst_mem, os->data_mem, os->core_count);
    if (os->cores[0] == RET_NULL)
        goto failure;
    os->stop = mfalse;
    return os; // we did everything correctly
failure:
    merry_os_destroy(os);
    return RET_NULL;
}

void merry_os_destroy(Merry *os)
{
    // free all the cores, memory, os and then exit
    if (surelyF(os == NULL))
        return;
    if (surelyT(os->data_mem != NULL))
        merry_memory_free(os->data_mem);
    if (surelyT(os->inst_mem != NULL))
        merry_memory_free(os->inst_mem);
    if (surelyT(os->_lock != NULL))
        merry_mutex_destroy(os->_lock);
    if (surelyT(os->_cond != NULL))
        merry_cond_destroy(os->_cond);
    if (surelyT(os->cores != NULL))
    {
        for (msize_t i = 0; i < os->core_count; i++)
        {
            if (surelyT(os->cores[i] != NULL))
                merry_core_destroy(os->cores[i]);
        }
        merry_free(os->cores);
    }
    merry_free(os);
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

_MERRY_INTERNAL_ void merry_os_boot_core(Merry *os, msize_t core_id, maddress_t start_addr)
{
    // this function's job is to boot up the core_id core and start execution
}

mptr_t merry_os_start_vm(mptr_t os)
{
    // this will start the OS
    Merry *master = (Merry *)os;

    return (mptr_t)master->ret;
}