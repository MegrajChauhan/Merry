#if defined(_WIN64)
#include "internals\merry_os.h"
#else
#include "internals/merry_os.h"
#endif
#include <stdatomic.h>

mret_t merry_os_init(mcstr_t _inp_file)
{
    // initialize the os
    // just 1 core
    // logger should be initialized before
    MerryInpFile *input = merry_read_file(_inp_file);
    if (input == RET_NULL)
    {
        // the input file was not read and we failed
        return RET_FAILURE;
    }
    // initialize the memory
    if ((os.data_mem = merry_dmemory_init_provided(input->_data, input->dpage_count)) == RET_NULL)
    {
        // grand failure
        merry_destory_reader(input);
        return RET_FAILURE;
    }
    // perform initialization for the inst mem as well. Unlike data memory, instruction page len cannot be 0
    // based on the size of the input file, the reader should be able to independently run in the background as it reads the input file
    // the reader doesn't concern itself with the OS and so it can run independently
    // all it has to do is map the necessary pages and return us a pointer
    // This way it can continue to read in the background and the VM can also continue without any problems
    if ((os.inst_mem = merry_memory_init_provided(input->_instructions, input->ipage_count)) == RET_NULL)
    {
        goto inp_failure;
    }
    // time for locks and mutexes
    if ((os._cond = merry_cond_init()) == RET_NULL)
        goto inp_failure;
    if ((os._lock = merry_mutex_init()) == RET_NULL)
        goto inp_failure;
    // don't forget to destory the reader
    if (merry_requestHdlr_init(_MERRY_REQUEST_QUEUE_LEN_, os._cond) == RET_FAILURE)
        goto inp_failure;
    maddress_t entry_point = input->entry_addr;
    merry_destory_reader(input);
    os.core_count = 1; // we will start with one core
    os.cores = (MerryCore **)malloc(sizeof(MerryCore *));
    if (os.cores == RET_NULL)
        goto failure;
    os.cores[0] = merry_core_init(os.inst_mem, os.data_mem, 0);
    if (os.cores[0] == RET_NULL)
        goto failure;
    os.stop = mfalse;
    os.cores[0]->pc = entry_point;
    os.core_threads = (MerryThread **)malloc(sizeof(MerryThread *)); // just 1 for now
    if (os.core_threads == RET_NULL)
        goto failure;
    if (merry_loader_init(2) == mfalse)
        goto failure;   // for now, 2
    return RET_SUCCESS; // we did everything correctly
failure:
    merry_os_destroy();
    return RET_FAILURE;
inp_failure:
    merry_os_destroy();
    merry_destory_reader(input);
    return RET_FAILURE;
}

void merry_os_destroy()
{
    // free all the cores, memory, os and then exit
    merry_dmemory_free(os.data_mem);
    merry_memory_free(os.inst_mem);
    merry_mutex_destroy(os._lock);
    merry_cond_destroy(os._cond);
    if (surelyT(os.cores != NULL))
    {
        for (msize_t i = 0; i < os.core_count; i++)
        {
            merry_core_destroy(os.cores[i]);
        }
        free(os.cores);
    }
    if (surelyT(os.core_threads != NULL))
    {
        for (msize_t i = 0; i < os.core_count; i++)
        {
            merry_thread_destroy(os.core_threads[i]);
        }
        free(os.core_threads);
    }
    merry_loader_close();
    merry_requestHdlr_destroy();
}

mret_t merry_os_boot_core(msize_t core_id, maddress_t start_addr)
{
    // this function's job is to boot up the core_id core and prepare it for execution
    os.cores[core_id]->pc = start_addr; // point to the starting address of the core
    // now start the core thread
    if ((os.core_threads[core_id] = merry_thread_init()) == RET_NULL)
        return RET_FAILURE;
    if (merry_create_detached_thread(os.core_threads[core_id], &merry_runCore, os.cores[core_id]) == RET_FAILURE)
        return RET_FAILURE;
    return RET_SUCCESS;
}

mret_t merry_os_add_core()
{
    // just add another core
    MerryThread **temp = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count + 1));
    if (temp == NULL)
        return RET_FAILURE; // we failed
    MerryCore **tempc = (MerryCore **)malloc(sizeof(MerryCore *) * (os.core_count + 1));
    if (tempc == NULL)
    {
        // we failed again
        free(temp);
        return RET_FAILURE;
    }
    tempc[os.core_count] = merry_core_init(os.inst_mem, os.data_mem, os.core_count);
    if (tempc[os.core_count] == RET_NULL)
    {
        free(temp);
        free(tempc);
        return RET_FAILURE;
    }
    // we have succeeded in add cores
    merry_mutex_lock(os._lock); // Safety for when request Pool is implemented
    for (msize_t i = 0; i <= os.core_count; i++)
    {
        temp[i] = os.core_threads[i];
        tempc[i] = os.cores[i];
    }
    // now free the old pointers and replace them
    free(os.core_threads);
    free(os.cores);
    os.core_threads = temp;
    os.cores = tempc;
    os.core_count++;
    merry_mutex_unlock(os._lock);
    return RET_SUCCESS;
}

_MERRY_INTERNAL_ void merry_os_prepare_for_exit()
{
    // prepare for termination
    // firstly tell all cores to shut down
    for (msize_t i = 0; i < os.core_count; i++)
    {
        atomic_exchange(&os.cores[i]->stop_running, mtrue);
    }
    // some cores may be waiting for their requests to be fulfilled
    merry_requestHdlr_kill_requests(); // kill all requests
    os.stop = mtrue;                   // done
}

/*From here the OS gets requests from the request handler and fulfills the request*/
_THRET_T_ merry_os_start_vm(mptr_t some_arg)
{
    // this will start the OS
    if (merry_os_boot_core(0, os.cores[0]->pc) != RET_SUCCESS)
        return (mptr_t)RET_FAILURE;
    // Core 0 is now up and running
    // The OS should be ready to handle requests
    MerryOSRequest current_req;
    mptr_t temp;
    while (os.stop == mfalse)
    {
        if (merry_requestHdlr_pop_request(&current_req) == mfalse)
        {
            // we have no requests to fulfill and so we goto sleep and wait for the request handler to wake us up
            merry_cond_wait(os._cond, os._lock);
        }
        else
        {
            // we have a request to fulfill
            if (_MERRY_REQUEST_INTERNAL_ERROR_(current_req.request_number))
            {
                merry_os_handle_internal_module_error(current_req.request_number);
                merry_os_prepare_for_exit(); // now since this is an error, we can't continue
            }
            else if (_MERRY_REQUEST_PROGRAM_ERROR_(current_req.request_number))
            {
                merry_os_handle_error(current_req.request_number); // this will handle all errors
                merry_os_prepare_for_exit();
            }
            else
            {
                switch (_MERRY_REQUEST_VALID_(current_req.request_number))
                {
                default:
                    // it is most likely an actual request
                    switch (current_req.request_number)
                    {
                    case _REQ_REQHALT: // halting request
                        merry_os_execute_request_halt(&os, &current_req); // this shouldn't generate any errors
                        break;
                    case _REQ_EXIT:
                        merry_os_prepare_for_exit();
                        os.ret = os.cores[current_req.id]->registers[Ma];
                        break;
                    case _REQ_NEWCORE:
                        merry_os_execute_request_new_core(&os, &current_req);
                        break;
                    case _REQ_DYNL:
                        merry_os_execute_request_dynl(&os, &current_req);
                        break;
                    case _REQ_DYNUL:
                        merry_os_execute_request_dynul(&os, &current_req);
                        break;
                    case _REQ_DYNCALL:
                        merry_os_execute_request_dyncall(&os, &current_req);
                        break;
                    case _REQ_FOPEN:
                        merry_os_execute_request_fopen(&os, &current_req);
                        break;
                    case _REQ_FCLOSE:
                        merry_os_execute_request_fclose(&os, &current_req);
                        break;
                    case _REQ_FREAD:
                        merry_os_execute_request_fread(&os, &current_req);
                        break;
                    case _REQ_FWRITE:
                        merry_os_execute_request_fwrite(&os, &current_req);
                        break;
                    case _REQ_FEOF:
                        merry_os_execute_request_feof(&os, &current_req);
                        break;
                    default:
                        fprintf(stderr, "Error: Unknown request code: '%llu' is not a valid request code", current_req.request_number);
                        break;
                    }
                }
            }
            // after the fulfillment of the request, wake up the core
            merry_cond_signal(current_req._wait_lock);
        }
    }
#if defined(_MERRY_HOST_OS_LINUX_)
    return (mptr_t)os.ret; // freeing the OS is the Main's Job
#elif defined(_MERRY_HOST_OS_WINDOWS_)
    return os.ret;
#endif
}

void merry_os_handle_error(merrot_t error)
{
    // this sets the return value for us
    os.ret = RET_FAILURE;
    switch (error)
    {
    case MERRY_MEM_ACCESS_ERROR:
        // this error generally implies that while reading from the provided address, the bytes were in different pages
        merry_mem_error("The provided address crossed the page boundary. Invalid addressing");
        break;
    case MERRY_MEM_INVALID_ACCESS:
        // this implies that the access is being requested for address that doesn't really exist
        merry_mem_error("Request to access memory that doesn't exist. Invalid address");
        break;
    case MERRY_DIV_BY_ZERO:
        merry_general_error("Div by zero", "Attempting to divide by zero");
        break;
    case MERRY_STACK_OVERFLOW:
        merry_general_error("Stack Overflow", "Cannot perform stack operations");
        break;
    case MERRY_STACK_UNDERFLOW:
        merry_general_error("Stack Underflow", "Cannot perform stack operations");
        break;
    case MERRY_CALL_DEPTH_REACHED:
        merry_general_error("RSA Depth reached", "Program reached the function call depth");
        break;
    case MERRY_INVALID_RETURN:
        merry_general_error("Bad instruction", "RET instruction without a CALL instruction; Not allowed");
        break;
    case MERRY_INVALID_VARIABLE_ACCESS:
        merry_general_error("Invalid BP offsetting", "The requested offset goes beyond the bounds of the stack");
        break;
    case MERRY_DYNL_FAILED:
        merry_general_error("Dynamic Loading Failed", "The requested library couldn't be loaded");
        break;
    case MERRY_DYNCALL_FAILED:
        merry_general_error("Dynamic Call Failed", "The dynamic function call failed; Maybe the name was incorrect?");
        break;
    case MERRY_FILEHANDLE_NULL:
        merry_general_error("Failed to perform file operations", "The file handle is NULL and trying to perform operations on a NULL handle is not a good idea.");
        break;
    case MERRY_INTERNAL_ERROR:
        merry_general_error("Internal Machine Error", "This isn't your fault most probably, try running the program again.");
        break;
    default:
        merry_error("Unknown error code: '%llu' is not a valid error code", error);
        break;
    }
}

void merry_os_handle_internal_module_error(merrot_t error_num)
{
    os.ret = RET_FAILURE;
    switch (error_num)
    {
    case _PANIC_REQBUFFEROVERFLOW:
        // either the program is deliberately trying to do this or the number is cores is just too much and requests to fast
        merry_internal_module_error("The request buffer hit maximum capacity. Cannot fulfill requests");
        break;
    }
}