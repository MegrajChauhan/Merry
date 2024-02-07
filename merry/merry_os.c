#include "internals/merry_os.h"

#include <stdatomic.h>

mret_t merry_os_init(mcstr_t _inp_file)
{
    // initialize the os
    // just 1 core
    // logger should be initialized before
    _log_(_OS_, "Initialization", "Intiializing the Manager");
    MerryInpFile *input = merry_read_file(_inp_file);
    if (input == RET_NULL)
    {
        // the input file was not read and we failed
        return RET_FAILURE;
    }
    // initialize the memory
    _log_(_OS_, "Intialization", "Intializing Data Memory");
    if ((os.data_mem = merry_dmemory_init_provided(input->_data, input->dpage_count)) == RET_NULL)
    {
        // grand failure
        _log_(_OS_, "Initialization Failure", "Failed to intiialize manager[Data Mem]");
        merry_destory_reader(input);
        return RET_FAILURE;
    }
    // perform initialization for the inst mem as well. Unlike data memory, instruction page len cannot be 0
    // based on the size of the input file, the reader should be able to independently run in the background as it reads the input file
    // the reader doesn't concern itself with the OS and so it can run independently
    // all it has to do is map the necessary pages and return us a pointer
    // This way it can continue to read in the background and the VM can also continue without any problems
    _log_(_OS_, "Intialization", "Intializing Instruction Memory");
    if ((os.inst_mem = merry_memory_init_provided(input->_instructions, input->ipage_count)) == RET_NULL)
    {
        _log_(_OS_, "Initialization Failure", "Failed to intiialize manager[Instruction Mem]");
        goto inp_failure;
    }
    // time for locks and mutexes
    _log_(_OS_, "Initialization", "Intializing necessary fields");
    if ((os._cond = merry_cond_init()) == RET_NULL)
        goto inp_failure;
    if ((os._lock = merry_mutex_init()) == RET_NULL)
        goto inp_failure;
    // don't forget to destory the reader
    if (merry_requestHdlr_init(_MERRY_REQUEST_QUEUE_LEN_, os._cond) == RET_FAILURE)
        goto inp_failure;
    merry_destory_reader(input);
    os.core_count = 1; // we will start with one core
    os.cores = (MerryCore **)malloc(sizeof(MerryCore *));
    if (os.cores == RET_NULL)
        goto failure;
    os.cores[0] = merry_core_init(os.inst_mem, os.data_mem, 0);
    if (os.cores[0] == RET_NULL)
        goto failure;
    os.stop = mfalse;
    os.core_threads = (MerryThread **)malloc(sizeof(MerryThread *)); // just 1 for now
    if (os.core_threads == RET_NULL)
        goto failure;
    // Merry *o = &os;
    if ((os.thPool = merry_init_thread_pool(_MERRY_THPOOL_LEN_)) == RET_NULL)
        goto failure;
    return RET_SUCCESS; // we did everything correctly
failure:
    _log_(_OS_, "Intialization Failure", "Failed to intialize the manager");
    merry_os_destroy();
    return RET_FAILURE;
inp_failure:
    _log_(_OS_, "Intialization Failure", "Failed to intialize the manager");
    merry_os_destroy();
    merry_destory_reader(input);
    return RET_FAILURE;
}

void merry_os_destroy()
{
    // free all the cores, memory, os and then exit
    _log_(_OS_, "Destroying", "Destroying the manager");
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
    merry_destroy_thread_pool(os.thPool);
    merry_requestHdlr_destroy();
}

// mret_t merry_os_mem_read_data(maddress_t address, mqptr_t store_in, msize_t core_id)
// {
//     // we can use different read/write based on the situation
//     // os will surely not be NULL
//     // store_in is either a reference or pointer to the core's register or something
//     // core_id is the unique value provided to cores by the OS which is used for identification
//     // since this request was made then the core is not NULL either
//     if (os.cores[core_id]->_is_private == mtrue)
//     {
//         // this core is assuring that it's data is not accessed by other cores
//         return merry_memory_read(os.data_mem, address, store_in);
//     }
//     else
//     {
//         return merry_manager_mem_read_data(address, store_in);
//     }
//     return RET_FAILURE;
// }

// mret_t merry_os_mem_write_data(maddress_t address, mqword_t to_store, msize_t core_id)
// {
//     if (os.cores[core_id]->_is_private == mtrue)
//     {
//         // this core is assuring that it's data is not accessed by other cores
//         return merry_memory_write(os.data_mem, address, to_store);
//     }
//     else
//     {
//         return merry_manager_mem_write_data(address, to_store);
//     }
//     return RET_FAILURE;
// }

// // helper: stops all of the cores
// _MERRY_INTERNAL_ void merry_os_stop_cores(Merry *os)
// {
//     merry_mutex_lock(os._lock);
//     if (os.core_count == 0)
//     {
//         // we have just 1 core
//         merry_mutex_lock(os.cores[0]->lock); // stop the core first
//         os.cores[0]->stop_running = mtrue;   // tell it to stop
//         merry_mutex_unlock(os.cores[0]->lock);
//     }
//     else
//     {
//         for (msize_t i = 0; i <= os.core_count; i++)
//         {
//             merry_mutex_lock(os.cores[i]->lock); // stop the core first
//             os.cores[i]->stop_running = mtrue;   // tell it to stop
//             merry_mutex_unlock(os.cores[i]->lock);
//         }
//     }
//     merry_mutex_unlock(os._lock);
// }

// _MERRY_INTERNAL_ void merry_os_stop_core(Merry *os, msize_t core_id)
// {
//     merry_mutex_lock(os._lock);
//     merry_mutex_lock(os.cores[core_id]->lock); // stop the core first
//     os.cores[core_id]->stop_running = mtrue;   // tell it to stop
//     merry_mutex_unlock(os.cores[core_id]->lock);
//     merry_mutex_unlock(os._lock);
//     if (os.core_count == 0)
//     {
//         // if count is 0 and we stopped the one already running
//         os.stop = mtrue;
//         os.ret = os.cores[core_id]->registers[Ma];
//         merry_cond_signal(os._cond);
//     }
// }

mret_t merry_os_boot_core(msize_t core_id, maddress_t start_addr)
{
    // this function's job is to boot up the core_id core and prepare it for execution
    os.cores[core_id]->pc = start_addr; // point to the starting address of the core
    // now start the core thread
    _llog_(_OS_, "Booting", "Booting core %d", core_id);
    if ((os.core_threads[core_id] = merry_thread_init()) == RET_NULL)
        return RET_FAILURE;
    if (merry_create_detached_thread(os.core_threads[core_id], &merry_runCore, os.cores[core_id]) == RET_FAILURE)
        return RET_FAILURE;
    _llog_(_OS_, "Booting", "Booting core %d succeeded", core_id);
    return RET_SUCCESS;
}

mret_t merry_os_add_core()
{
    // just add another core
    MerryThread **temp = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count + 1));
    if (temp == NULL)
        return RET_FAILURE; // we failed
    MerryCore **tempc = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count + 1));
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
    _log_(_OS_, "Exiting", "Preparing for exit");
    for (msize_t i = 0; i < os.core_count; i++)
    {
        // merry_mutex_lock(os.cores[i]->lock);
        // os.cores[i]->stop_running = mtrue; // stop
        atomic_exchange(&os.cores[i]->stop_running, mtrue);
        // we may add furthur functionalities here such as the ability to know the value of different registers at the time of termination
        // merry_mutex_unlock(os.cores[i]->lock);
    }
    // some cores may be waiting for their requests to be fulfilled
    merry_requestHdlr_kill_requests(); // kill all requests
    os.stop = mtrue;                   // done
}

/*From here the OS gets requests from the request handler and fulfills the request*/
mptr_t merry_os_start_vm(mptr_t some_arg)
{
    // this will start the OS
    _log_(_OS_, "Starting Manager", "Manager thread running");
    // Merry *x = &os; // temp
    _log_(_OS_, "STARTING CORE 0", "Attempting to start core 0");
    if (merry_os_boot_core(0, 0) != RET_SUCCESS)
        return (mptr_t)RET_FAILURE;
    // Core 0 is now up and running
    // The OS should be ready to handle requests
    MerryOSRequest current_req;
    mptr_t temp;
    _log_(_OS_, "STARTING EXECUTION", "Manager is entering the request handling loop");
    while (os.stop == mfalse)
    {
        // there is no need to lock the OS now so maybe we won't need the Mutex locks
        // We can implement the OS to be capable of handling various services at once
        // for eg: It could provide input service for one core while providing output service for another core simultaneoulsy
        // This would utilize the OS's full potential and the time wasted by core's waiting would be eliminated
        if (merry_requestHdlr_pop_request(&current_req) == mfalse)
        {
            // we have no requests to fulfill and so we goto sleep and wait for the request handler to wake us up
            _log_(_OS_, "Waiting", "Manager waiting for requests");
            merry_cond_wait(os._cond, os._lock);
        }
        else
        {
            // we have a request to fulfill
            if (_MERRY_REQUEST_INTERNAL_ERROR_(current_req.request_number))
            {
                _llog_(_OS_, "Error", "Internal Error Detected: Error code %d", current_req.request_number);
                merry_os_handle_internal_module_error(current_req.request_number);
                merry_os_prepare_for_exit(); // now since this is an error, we can't continue
            }
            else if (_MERRY_REQUEST_PROGRAM_ERROR_(current_req.request_number))
            {
                _llog_(_OS_, "Error", "Program generated error: Error code %d", current_req.request_number);
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
                        _llog_(_OS_, "REQ", "Halt request received from core ID %lu", current_req.id);
                        merry_os_execute_request_halt(&os, &current_req); // this shouldn't generate any errors
                        break;
                    case _REQ_EXIT:
                        _llog_(_OS_, "REQ", "Exit request received from core ID %lu", current_req.id);
                        merry_os_prepare_for_exit();
                        os.ret = os.cores[current_req.id]->registers[Ma];
                        break;
                    case _REQ_NEWCORE:
                        _llog_(_OS_, "REQ", "New core creation request received from core ID %lu", current_req.id);
                        merry_os_execute_request_new_core(&os, &current_req);
                        break;
                    default:
                        /// NOTE: this will come in handy when we implement some built-in syscalls and the program provides invalid syscalls
                        merry_error("Unknown request code: '%llu' is not a valid request code", current_req.request_number);
                        break;
                    }
                }
            }
            // after the fulfillment of the request, wake up the core
            _llog_(_OS_, "REQ_FULFILLED", "Core ID %lu request %lu fulfilled, Waking up", current_req.id, current_req.request_number);
            merry_cond_signal(current_req._wait_lock);
        }
    }
    _llog_(_OS_, "EXIT", "Manager terminating with exit code %ld", os.ret);
    return (mptr_t)os.ret; // freeing the OS is the Main's Job
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