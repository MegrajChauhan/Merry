#include "internals/merry_os.h"

Merry *merry_os_init(mcstr_t _inp_file)
{
    // firstly initialize the request handler
    // merry_requestHdlr_init();
    // initialize the os
    Merry *os = (Merry *)malloc(sizeof(Merry));
    if (os == RET_NULL)
        return RET_NULL; // we failed
    // just 1 core
    // os->cores = (Merry **)malloc(sizeof(Merry *));
    if (os->cores == RET_NULL)
    {
        free(os);
        return RET_NULL;
    }
    // we need memory to be initialized
    MerryInpFile *input = merry_read_file(_inp_file);
    if (input == RET_NULL)
    {
        // the input file was not read and we failed
        free(os);
        return RET_NULL;
    }
    // initialize the memory
    if ((os->data_mem = merry_memory_init_provided(input->_data, input->dpage_count)) == RET_NULL)
    {
        // grand failure
        free(os);
        merry_destory_reader(input);
        return RET_NULL;
    }
    // perform initialization for the inst mem as well. Unlike data memory, instruction page len cannot be 0
    // based on the size of the input file, the reader should be able to independently run in the background as it reads the input file
    // the reader doesn't concern itself with the OS and so it can run independently
    // all it has to do is map the necessary pages and return us a pointer
    // This way it can continue to read in the background and the VM can also continue without any problems
    if ((os->inst_mem = merry_memory_init_provided(input->_instructions, input->ipage_count)) == RET_NULL)
    {
        merry_memory_free(os->data_mem);
        goto inp_failure;
    }
    // time for locks and mutexes
    if ((os->_cond = merry_cond_init()) == RET_NULL)
        goto inp_failure;
    if ((os->_lock = merry_mutex_init()) == RET_NULL)
        goto inp_failure;
    // don't forget to destory the reader
    merry_destory_reader(input);
    os->core_count = 0; // we will start with one core
    os->cores = (MerryCore **)malloc(sizeof(MerryCore *));
    if (os->cores == RET_NULL)
        goto failure;
    os->cores[0] = merry_core_init(os->inst_mem, os->data_mem, os, os->core_count);
    if (os->cores[0] == RET_NULL)
        goto failure;
    os->stop = mfalse;
    os->core_threads = (MerryThread **)malloc(sizeof(MerryThread *)); // just 1 for now
    if (os->core_threads == RET_NULL)
        goto failure;
    return os; // we did everything correctly
failure:
    merry_os_destroy(os);
    return RET_NULL;
inp_failure:
    merry_os_destroy(os);
    merry_destory_reader(input);
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
        free(os->cores);
    }
    if (surelyT(os->core_threads != NULL))
    {
        for (msize_t i = 0; i < os->core_count; i++)
        {
            if (surelyT(os->core_threads[i] != NULL))
                merry_thread_destroy(os->core_threads[i]);
        }
        free(os->core_threads);
    }
    merry_requestHdlr_destroy();
    free(os);
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

// // helper: stops all of the cores
// _MERRY_INTERNAL_ void merry_os_stop_cores(Merry *os)
// {
//     merry_mutex_lock(os->_lock);
//     if (os->core_count == 0)
//     {
//         // we have just 1 core
//         merry_mutex_lock(os->cores[0]->lock); // stop the core first
//         os->cores[0]->stop_running = mtrue;   // tell it to stop
//         merry_mutex_unlock(os->cores[0]->lock);
//     }
//     else
//     {
//         for (msize_t i = 0; i <= os->core_count; i++)
//         {
//             merry_mutex_lock(os->cores[i]->lock); // stop the core first
//             os->cores[i]->stop_running = mtrue;   // tell it to stop
//             merry_mutex_unlock(os->cores[i]->lock);
//         }
//     }
//     merry_mutex_unlock(os->_lock);
// }

// _MERRY_INTERNAL_ void merry_os_stop_core(Merry *os, msize_t core_id)
// {
//     merry_mutex_lock(os->_lock);
//     merry_mutex_lock(os->cores[core_id]->lock); // stop the core first
//     os->cores[core_id]->stop_running = mtrue;   // tell it to stop
//     merry_mutex_unlock(os->cores[core_id]->lock);
//     merry_mutex_unlock(os->_lock);
//     if (os->core_count == 0)
//     {
//         // if count is 0 and we stopped the one already running
//         os->stop = mtrue;
//         os->ret = os->cores[core_id]->registers[Ma];
//         merry_cond_signal(os->_cond);
//     }
// }

_MERRY_ALWAYS_INLINE mret_t merry_os_boot_core(Merry *os, msize_t core_id, maddress_t start_addr)
{
    // this function's job is to boot up the core_id core and prepare it for execution
    os->cores[core_id]->pc = start_addr; // point to the starting address of the core
    // now start the core thread
    if (os->core_threads[core_id] == NULL)
    {
        if ((os->core_threads[core_id] = merry_thread_init()) == RET_NULL)
            return RET_FAILURE;
    }
    if (merry_create_detached_thread(os->core_threads[core_id], &merry_runCore, os->cores[core_id]) == RET_FAILURE)
        return RET_FAILURE;
    return RET_SUCCESS;
}

/*From here the OS gets requests from the request handler and fulfills the request*/
mptr_t merry_os_start_vm(mptr_t os)
{
    // this will start the OS
    Merry *master = (Merry *)os;
    if (merry_os_boot_core(master, 0, 0) != RET_SUCCESS)
        return (mptr_t)RET_FAILURE;
    /*Logic for request handling*/
    return (mptr_t)master->ret; // freeing the OS is the Main's Job
}

void merry_os_handle_error(Merry *os, MerryError error)
{
    // since error was encountered
    // firstly stop all the cores
    merry_os_stop_cores(os);
    // now that nothing is using the OS we can proceed
    os->ret = RET_FAILURE;
    switch (error)
    {
    case MERRY_MEM_ACCESS_ERROR:
        // this error generally implies that while reading from the provided address, the bytes were in different pages
        fprintf(stderr, "Access Fault: Memory access fault. Invalid addressing[Address maybe misaligned]\n");
        break;
    case MERRY_MEM_INVALID_ACCESS:
        // this implies that the access is being requested for address that doesn't really exist
        fprintf(stderr, "Memory Fault: Address is out of range. Cannot access beyond the memory.\n");
        break;
    default:
        break;
    }
    merry_cond_signal(os->_cond); // signal the OS
}