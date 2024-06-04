#include "os/merry_os.h"

mret_t merry_os_init(mcstr_t _inp_file, char **options, msize_t count)
{
    // initialize the os
    os.reader = merry_init_reader(_inp_file);
    if (os.reader == RET_NULL)
        return RET_FAILURE;
    // initialize the memory
    if (merry_reader_read_file(os.reader) == RET_FAILURE)
    {
        merry_destroy_reader(os.reader);
        return RET_FAILURE;
    }
    if ((os.data_mem = merry_dmemory_init_provided(os.reader->data, os.reader->data_page_count)) == RET_NULL)
    {
        merry_destroy_reader(os.reader);
        return RET_FAILURE;
    }
    // we need to put the options into the memory
    msize_t len = 0;
    for (msize_t i = 0; i < count; i++)
    {
        len += strlen(options[i]);
    }
    if (count > 0)
    {
        mbptr_t _ = &(options[0][0]);
        if (merry_dmemory_write_bytes_maybe_over_multiple_pages(os.data_mem, os.reader->data_len + 1, len, _) == RET_FAILURE)
        {
            // we don't have enough memory
            if (merry_dmemory_add_new_page(os.data_mem) == RET_FAILURE)
                goto failure;
            // try again and this time it should work, if it doesn't just quit
            if (merry_dmemory_write_bytes_maybe_over_multiple_pages(os.data_mem, os.reader->data_len + 1, len, _) == RET_FAILURE)
                goto failure;
        }
    }
    // we have that in memory now
    msize_t _t = os.reader->data_len;
    // perform initialization for the inst mem as well. Unlike data memory, instruction page len cannot be 0
    if ((os.inst_mem = merry_memory_init_provided(os.reader->inst.instructions, os.reader->inst.inst_page_count)) == RET_NULL)
        goto failure;
    // initialize all the cores
    os.cores = (MerryCore **)malloc(sizeof(MerryCore *) * os.reader->eat.eat_entry_count);
    if (os.cores == NULL)
        goto failure;
    os.core_count = 0;
    for (msize_t i = 0; i < os.reader->eat.eat_entry_count; i++, os.core_count++)
    {
        if ((os.cores[i] = merry_core_init(os.inst_mem, os.data_mem, os.core_count)) == RET_NULL)
            goto failure;
    }
    // time for locks and mutexes
    if ((os._cond = merry_cond_init()) == RET_NULL)
        goto failure;
    if ((os._lock = merry_mutex_init()) == RET_NULL)
        goto failure;
    if (merry_requestHdlr_init(_MERRY_REQUEST_QUEUE_LEN_, os._cond) == RET_FAILURE)
        goto failure;
    os.cores[0]->registers[Mm1] = count;        // Mm1 will have the number of options
    os.cores[0]->registers[Md] = _t + 1;        // Md will have the address to the first byte
    os.cores[0]->registers[Mm5] = len + _t + 1; // Mm5 contains the address of the first byte that is free and can be manipulated by the program
    os.stop = mfalse;
    os.core_threads = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count + 1));
    if (os.core_threads == RET_NULL)
        goto failure;
    if (merry_loader_init(2) == mfalse)
        goto failure; // for now, 2
    os.active_core_count = 0;
    return RET_SUCCESS; // we did everything correctly
failure:
    merry_os_destroy();
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
    merry_destroy_reader(os.reader);
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
    os.active_core_count++;
    return RET_SUCCESS;
}

mret_t merry_os_add_core()
{
    // just add another core
    merry_mutex_lock(os._lock);
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
    for (msize_t i = 0; i < os.core_count; i++)
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
    for (msize_t i = 0; i < os.core_count; i++)
    {
        if (merry_os_boot_core(i, os.reader->eat.EAT[i]) != RET_SUCCESS)
        {
            os.core_count = i - 1;
            merry_os_prepare_for_exit();
            return (mptr_t)RET_FAILURE;
        }
    }
    MerryOSRequest current_req;
    mptr_t temp;
    while (os.stop == mfalse)
    {
        if (os.active_core_count == 0)
            break;
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
                    case _REQ_REQHALT:                                    // halting request
                        merry_os_execute_request_halt(&os, &current_req); // this shouldn't generate any errors
                        os.active_core_count--;
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
                    case _REQ_REWIND:
                        merry_os_execute_request_rewind(&os, &current_req);
                        break;
                    case _REQ_MEM:
                        merry_os_execute_request_mem(&os, &current_req);
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
    return (mptr_t)(&os.ret); // freeing the OS is the Main's Job
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

_os_exec_(halt)
{
    // here we have to stop the core first
    // since the core is waiting, we don't have to worry about mutex locks.
    // since the core's decoder won't mess with other fields of the core, we can freely make changes
    os->cores[request->id]->stop_running = mtrue; // this will automatically halt the core's decoder as well
    if (os->core_count == 1)                      // I am an idiot
    {
        // we had only one core to begin with then stop any further execution
        os->stop = mtrue;
        // the core that makes this request should have the return value in Ma register
        os->ret = os->cores[request->id]->registers[Ma];
    }
    return RET_SUCCESS; // for mitigating compiler's warning
}

_os_exec_(new_core)
{
    // generate a new core
    if (merry_os_add_core() == RET_FAILURE)
    {
        // let the core know that its request was a failure
        os->cores[request->id]->registers[Ma] = 1; // Ma should contain the address and it will be updated with the result of the request
    }
    else
    {
        os->cores[request->id]->registers[Ma] = merry_os_boot_core(os->core_count - 1, os->cores[request->id]->registers[Ma]);
    }
    return RET_SUCCESS; // for now
}

_os_exec_(dynl)
{
    // the address to the name of the library must be in the Ma register
    // if the name is not and it is invalidly placed, the host will throw a segfault
    mbptr_t name = merry_dmemory_get_byte_address(os->data_mem, os->cores[request->id]->registers[Ma]);
    if (name == RET_NULL)
    {
        merry_requestHdlr_panic(os->data_mem->error);
        return RET_FAILURE;
    }
    if (merry_loader_loadLib(name, &os->cores[request->id]->registers[Mb]) == mfalse)
    {
        merry_requestHdlr_panic(MERRY_DYNL_FAILED);
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

_MERRY_ALWAYS_INLINE_ inline _os_exec_(dynul)
{
    merry_loader_unloadLib(os->cores[request->id]->registers[Mb]);
    return RET_SUCCESS;
}

_os_exec_(dyncall)
{
    dynfunc_t function;
    mqptr_t param = merry_dmemory_get_qword_address(os->data_mem, os->cores[request->id]->registers[Mc]);
    mbptr_t func_name = merry_dmemory_get_byte_address(os->data_mem, os->cores[request->id]->registers[Ma]);
    if (param == NULL || func_name == NULL)
    {
        merry_requestHdlr_panic(MERRY_DYNCALL_FAILED);
        return RET_FAILURE;
    }
    if ((function = merry_loader_getFuncSymbol(os->cores[request->id]->registers[Mb], func_name)) == RET_NULL)
    {
        merry_requestHdlr_panic(MERRY_DYNCALL_FAILED);
        return RET_FAILURE;
    }
    os->cores[request->id]->registers[Ma] = function(param);
    return RET_SUCCESS;
}

/// NOTE: It is the program's job to not mess with the file handle. The VM might crash or undefined behaviour could occur. Since there is no direct way for the VM to know that the file handle
// is indeed valid, it is the program's job to keep the handle safe. The same can be applied to the handle returned by dynl

_os_exec_(fopen)
{
    // the address to the filename should be in Ma
    // the file opening mode should be in the lower 2 bits of the Mb register
    // the handle will be returned in the Mb register and the return value in the Ma register
    // the VM won't exit on open failure
    // the filename must be null terminated
    mbptr_t file_name = merry_dmemory_get_byte_address(os->data_mem, os->cores[request->id]->registers[Ma]);
    msize_t open_mode = os->cores[request->id]->registers[Mb] & 0b11;
    mqword_t _fd = 0;
    if (merry_open_new_file(file_name, open_mode, &_fd) == RET_FAILURE)
    {
        os->cores[request->id]->registers[Ma] = 1;
        return RET_FAILURE;
    }
    os->cores[request->id]->registers[Mb] = (mqword_t)_fd; // store the handle
    os->cores[request->id]->registers[Ma] = 0;             // representing success
    return RET_SUCCESS;
}

_os_exec_(fclose)
{
    // the handle to the file to close must be in the Mb register
    mqword_t handle = os->cores[request->id]->registers[Mb];
    if (handle == (mqword_t)-1)
    {
        merry_requestHdlr_panic(MERRY_FILEHANDLE_NULL);
        return RET_FAILURE;
    }
    merry_close_file(&handle);
    return RET_SUCCESS;
}

_os_exec_(fread)
{
    // if the file handle is NULL, error is thrown
    // The file handle is in the Mb register
    // The address to store the read contents should be in the Ma register
    // The number of bytes to read should be in the Mc register
    // The number of bytes read will be in the Ma register
    mqword_t handle = os->cores[request->id]->registers[Mb];
    if (handle == (mqword_t)-1)
    {
        merry_requestHdlr_panic(MERRY_FILEHANDLE_NULL);
        return RET_FAILURE;
    }
    mbptr_t store_in = merry_dmemory_get_byte_address(os->data_mem, os->cores[request->id]->registers[Ma]);
    register msize_t bytes_to_read = os->cores[request->id]->registers[Mc];
    os->cores[request->id]->registers[Ma] = merry_read_from_file(&handle, store_in, bytes_to_read);
    return RET_SUCCESS;
}

_os_exec_(fwrite)
{
    // exactly the same but write is performed instead
    mqword_t handle = os->cores[request->id]->registers[Mb];
    if (handle == (mqword_t)-1)
    {
        merry_requestHdlr_panic(MERRY_FILEHANDLE_NULL);
        return RET_FAILURE;
    }
    mbptr_t to_write = merry_dmemory_get_byte_address(os->data_mem, os->cores[request->id]->registers[Ma]);
    register msize_t bytes_to_read = os->cores[request->id]->registers[Mc];
    os->cores[request->id]->registers[Ma] = merry_write_to_file(&handle, to_write, bytes_to_read);
    return RET_SUCCESS;
}

_os_exec_(feof)
{
    // handle in Mb register
    // nonzero in Ma if eof else 0
    mqword_t handle = os->cores[request->id]->registers[Mb];
    if (handle == (mqword_t)-1)
    {
        merry_requestHdlr_panic(MERRY_FILEHANDLE_NULL);
        return RET_FAILURE;
    }
    mbyte_t tmp;
    os->cores[request->id]->registers[Ma] = (merry_read_from_file(&handle, &tmp, 1) == 0) ? 0 : 1;
    return RET_SUCCESS;
}

_os_exec_(fseek)
{
    // Mb contains the handle
    // Ma will contain the return value, 0 for success
    // Mc contains the offset from which to seek
    // Md contains the offset to which to seek
    mqword_t handle = os->cores[request->id]->registers[Mb];
    register mqword_t whence = os->cores[request->id]->registers[Mc];
    register mqword_t offset = os->cores[request->id]->registers[Md];
    if ((mqptr_t)handle == NULL)
    {
        merry_requestHdlr_panic(MERRY_FILEHANDLE_NULL);
        return RET_FAILURE;
    }
    mret_t _res = merry_seek_in_file(&handle, offset, whence);
    os->cores[request->id]->registers[Ma] = _res == RET_FAILURE ? 1 : 0;
    return RET_SUCCESS;
}

_os_exec_(ftell)
{
    // Mb contains the handle
    // Ma contains the offset
    // register mqword_t handle = os->cores[request->id]->registers[Mb];
    // if ((mqptr_t)handle == NULL)
    // {
    //     merry_requestHdlr_panic(MERRY_FILEHANDLE_NULL);
    //     return RET_FAILURE;
    // }
    // os->cores[request->id]->registers[Ma] = ftell((FILE *)handle);

    // We don't know what to do right now with this

    return RET_SUCCESS;
}

_os_exec_(rewind)
{
    // Mb contains the handle
    mqword_t handle = os->cores[request->id]->registers[Mb];
    if (handle == (mqword_t)-1)
    {
        merry_requestHdlr_panic(MERRY_FILEHANDLE_NULL);
        return RET_FAILURE;
    }
    merry_seek_in_file(&handle, 0, 0);
    return RET_SUCCESS;
}

_os_exec_(mem)
{
    // each time this request is made, a new page is allocated
    // the starting address of the new page will be returned in Mb register
    // if failed, Ma will contain 1 else 0
    if (merry_dmemory_add_new_page(os->data_mem) == RET_FAILURE)
    {
        os->cores[request->id]->registers[Ma] = 1;
        return RET_FAILURE;
    }
    os->cores[request->id]->registers[Mb] = (os->data_mem->number_of_pages - 1) * _MERRY_MEMORY_ADDRESSES_PER_PAGE_ + 1;
    return RET_SUCCESS;
}
