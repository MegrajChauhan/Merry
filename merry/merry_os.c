#include "merry_os.h"

mret_t merry_os_init(mcstr_t _inp_file, char **options, msize_t count, mbool_t _wait_for_conn)
{
    // initialize the os
    os.listener_stopped = os.sender_stopped = mtrue;
    os._os_id = 0;
    if (merry_loader_init(_MERRY_INITIAL_DYNLOAD_COUNT_) == mfalse)
        return RET_FAILURE;
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
    os.core_count = os.reader->eat.eat_entry_count;
    for (msize_t i = 0; i < os.reader->eat.eat_entry_count; i++)
    {
        if ((os.cores[i] = merry_core_init(os.inst_mem, os.data_mem, i)) == RET_NULL)
            goto failure;
    }
    // time for locks and mutexes
    if ((os._cond = merry_cond_init()) == RET_NULL)
        goto failure;
    if ((os._lock = merry_mutex_init()) == RET_NULL)
        goto failure;
    if (merry_requestHdlr_init(_MERRY_REQUEST_QUEUE_LEN_, os._cond) == RET_FAILURE)
        goto failure;
    os.listener_running = mfalse;
    os.sender_running = mfalse;
    os.listener_stopped = mtrue;
    os.sender_stopped = mtrue;
    /// NOTE: Don't mind the nested if-conditions if you will.
    if (os.reader->de_flag == mtrue)
        merry_os_start_dbg(_wait_for_conn, _MERRY_DEFAULT_LISTEN_PORT_, _MERRY_DEFAULT_SEND_PORT_);
    os.cores[0]->registers[Mm1] = count;    // Mm1 will have the number of options
    os.cores[0]->registers[Md] = _t;        // Md will have the address to the first byte
    os.cores[0]->registers[Mm5] = len + _t; // Mm5 contains the address of the first byte that is free and can be manipulated by the program
    os.cores[0]->registers[Mm4] = _MERRY_HOST_ID_ARCH_;
    os.cores[0]->registers[Mm3] = _MERRY_HOST_ID_OS_;
    os.cores[0]->registers[Mm2] = os.data_mem->number_of_pages; // Number of pages that the program can manipulate
    os.stop = mfalse;
    os.core_threads = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count));
    if (os.core_threads == RET_NULL)
        goto failure;
    os.active_core_count = 0;
    os.err_core_id = 0;
    os.ret = _MERRY_EXIT_SUCCESS_;
    os._subsystem_running = mfalse;
    os._subsystem_failure = mfalse;
    merry_trap_install(); // for now, the failure of this doesn't matter
    return RET_SUCCESS;   // we did everything correctly
failure:
    merry_os_destroy();
    return RET_FAILURE;
}

mret_t merry_os_start_subsys()
{
    if ((os.os_pipe = merry_open_pipe()) == RET_NULL)
        goto failure;
    if (merry_init_subsys(_MERRY_SUBSYS_LEN_) == RET_FAILURE)
    {
        merry_destroy_pipe(os.os_pipe);
        goto failure;
    }
    if ((os.subsys_thread = merry_thread_init()) == RET_NULL)
    {
        merry_destroy_pipe(os.os_pipe);
        merry_destroy_subsys();
        goto failure;
    }
    merry_subsys_add_ospipe(os.os_pipe);
    if (merry_create_detached_thread(os.subsys_thread, &merry_subsys_main, NULL) == RET_FAILURE)
    {
        merry_destroy_pipe(os.os_pipe);
        merry_destroy_subsys();
        merry_thread_destroy(os.subsys_thread);
        goto failure;
    }
    os._subsystem_running = mtrue;
    return RET_SUCCESS;
failure:
    os._subsystem_failure = mtrue;
    return RET_FAILURE;
}

mret_t merry_os_init_reader_provided(MerryReader *r, msize_t iport, msize_t oport)
{
    // we need to put the options into the memory
    os._os_id = os._os_id + 1;
    os.reader = r;
    if ((os.data_mem = merry_dmemory_init_provided(os.reader->data, os.reader->data_page_count)) == RET_NULL)
    {
        merry_destroy_reader(os.reader);
        return RET_FAILURE;
    }
    if ((os.inst_mem = merry_memory_init_provided(os.reader->inst.instructions, os.reader->inst.inst_page_count)) == RET_NULL)
        goto failure;
    if (merry_loader_init(_MERRY_INITIAL_DYNLOAD_COUNT_) == mfalse)
        return RET_FAILURE;
    msize_t _t = os.reader->data_len;
    // initialize all the cores
    os.cores = (MerryCore **)malloc(sizeof(MerryCore *) * os.reader->eat.eat_entry_count);
    if (os.cores == NULL)
        goto failure;
    os.core_count = os.reader->eat.eat_entry_count;
    for (msize_t i = 0; i < os.reader->eat.eat_entry_count; i++)
    {
        if ((os.cores[i] = merry_core_init(os.inst_mem, os.data_mem, i)) == RET_NULL)
            goto failure;
    }
    // time for locks and mutexes
    if ((os._cond = merry_cond_init()) == RET_NULL)
        goto failure;
    if ((os._lock = merry_mutex_init()) == RET_NULL)
        goto failure;
    if (merry_requestHdlr_init(_MERRY_REQUEST_QUEUE_LEN_, os._cond) == RET_FAILURE)
        goto failure;
    if (os.reader->dfe_flag == mtrue)
        merry_os_start_dbg(os.reader->dfw_flag, iport, oport);
    os.stop = mfalse;
    os.core_threads = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count));
    if (os.core_threads == RET_NULL)
        goto failure;
    os.active_core_count = 0;
    // This init is done with a child process
    // let them know that they are a child process
    os.cores[0]->registers[Ma] = 1;
    return RET_SUCCESS; // we did everything correctly
failure:
    merry_os_destroy();
    return RET_FAILURE;
}

void merry_os_start_dbg(mbool_t _flag, msize_t in_port, msize_t out_port)
{
    /// NOTE: Don't mind the nested if-conditions if you will.
    if (os.reader->de_flag == mtrue)
    {
        // Initialize debugger threads
        os.listener = merry_init_listener(_flag, in_port);
        if (os.listener != RET_NULL)
        {
            os.sender = merry_init_sender(_flag, out_port);
            if (os.sender != RET_NULL)
            {
                os.listener_th = merry_thread_init();
                os.sender_th = merry_thread_init();
                if (os.listener_th == NULL || os.sender_th == NULL)
                {
                    merry_destroy_sender(os.sender);
                    merry_destroy_listener(os.listener);
                    rlog("Internal Error: DE flag provided but couldn't start the debugger threads.\n", NULL);
                }
                else
                {
                    if (merry_create_detached_thread(os.listener_th, &merry_start_listening, os.listener) != RET_FAILURE)
                    {
                        if (merry_create_detached_thread(os.sender_th, &merry_start_sending, os.sender) == RET_FAILURE)
                        {
                            atomic_store(&os.listener->stop, mtrue);
                            rlog("Internal Error: DE flag provided but couldn't start the debugger threads.\n", NULL);
                        }
                    }
                }
            }
            else
            {
                merry_destroy_listener(os.listener);
                rlog("Internal Error: DE flag provided but couldn't start the debugger threads.\n", NULL);
            }
            while (_flag == mtrue && (os.listener_running != mtrue || os.sender_running != mtrue))
            {
            }
            os.listener_stopped = os.sender_stopped = mfalse;
        }
        else
            rlog("Internal Error: DE flag provided but couldn't start the debugger threads.\n", NULL);
    }
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
            merry_core_destroy(os.cores[i], mtrue);
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
    if (os._subsystem_running == mtrue)
    {
        merry_thread_destroy(os.subsys_thread);
        merry_destroy_subsys();
        merry_destroy_pipe(os.os_pipe);
    }
    merry_destroy_sender(os.sender);
    merry_destroy_listener(os.listener);
    merry_thread_destroy(os.listener_th);
    merry_thread_destroy(os.sender_th);
    merry_destroy_reader(os.reader);
    merry_loader_close();
    merry_requestHdlr_destroy();
}

/*UN-USED OR DEPRECATED*/
void merry_os_new_proc_cleanup()
{
    merry_mutex_destroy(os._lock);
    merry_cond_destroy(os._cond);
    if (surelyT(os.cores != NULL))
    {
        for (msize_t i = 0; i < os.core_count; i++)
        {
            merry_core_destroy(os.cores[i], mfalse);
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
    merry_cleanup_sender(os.sender);
    os.sender = NULL;
    merry_destroy_listener(os.listener);
    os.listener = NULL;
    merry_thread_destroy(os.listener_th);
    merry_thread_destroy(os.sender_th);
    os.listener_th = os.sender_th = NULL;
    os.listener_stopped = os.sender_stopped = mtrue;
    merry_requestHdlr_destroy();
}

mret_t merry_os_boot_core(msize_t core_id, maddress_t start_addr)
{
    // this function's job is to boot up the core_id core and prepare it for execution
    os.cores[core_id]->pc = start_addr; // point to the starting address of the core
    os.cores[core_id]->entry_addr = start_addr;
    // now start the core thread
    if ((os.core_threads[core_id] = merry_thread_init()) == RET_NULL)
    {
        return RET_FAILURE;
    }
    if (merry_create_detached_thread(os.core_threads[core_id], &merry_runCore, os.cores[core_id]) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    os.active_core_count++;
    merry_os_notify_dbg(_NEW_CORE_, core_id, 0);
    return RET_SUCCESS;
}

mret_t merry_os_add_core()
{
    // just add another core
    MerryCore *new_core = merry_core_init(os.inst_mem, os.data_mem, os.core_count);
    if (new_core == RET_NULL)
    {
        goto _err;
    }
    MerryThread *th = merry_thread_init();
    if (th == RET_NULL)
    {
        merry_core_destroy(new_core, mtrue);
        goto _err;
    }
    MerryThread **temp = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count));
    if (temp == NULL)
    {
        merry_core_destroy(new_core, mtrue);
        merry_thread_destroy(th);
        goto _err;
    }
    MerryCore **tempc = (MerryCore **)malloc(sizeof(MerryCore *) * (os.core_count));
    if (tempc == NULL)
    {
        // we failed again
        merry_core_destroy(new_core, mtrue);
        merry_thread_destroy(th);
        free(temp);
        goto _err;
    }
    // we have succeeded in add cores
    for (msize_t i = 0; i < os.core_count; i++)
    {
        temp[i] = os.core_threads[i];
        tempc[i] = os.cores[i];
    }
    temp[os.core_count] = th;
    tempc[os.core_count] = new_core;
    // now free the old pointers and replace them
    free(os.core_threads);
    free(os.cores);
    os.core_threads = temp;
    os.cores = tempc;
    os.core_count++;
    return RET_SUCCESS;
_err:

    return RET_FAILURE;
}

_MERRY_INTERNAL_ void merry_os_notify_subsys()
{
    char buf = 1;
    write(os.os_pipe->_wclosed, &buf, 1); // wake it up
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
    if (os.listener_running == mtrue)
        atomic_exchange(&os.listener->stop, mtrue);
    merry_os_notify_dbg(_TERMINATING_, os._os_id, 0);
    if (os.sender_running == mtrue)
    {
        atomic_exchange(&os.sender->stop, mtrue);
        if (os.sender->queue->data_count == 0)
            merry_cond_signal(os.sender->cond);
    }
    if (os._subsystem_running == mtrue && os._subsystem_failure == mfalse)
    {
        char _send = _SUBSYS_SHUTDOWN;
        write(os.os_pipe->_write_fd, &_send, 1);
    }
    os.stop = mtrue; // done
}

/*From here the OS gets requests from the request handler and fulfills the request*/
_THRET_T_ merry_os_start_vm(mptr_t some_arg)
{
    // this will start the OS
    for (msize_t i = 0; i < os.core_count; i++)
    {
        if (merry_os_boot_core(i, os.reader->eat.EAT[i]) != RET_SUCCESS)
        {
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
            if (_MERRY_REQUEST_OTHER_(current_req.request_number))
                merry_os_handle_others(current_req.request_number, 0);
            else if (_MERRY_REQUEST_INTERNAL_ERROR_(current_req.request_number))
            {
                os.err_core_id = current_req.id;
                merry_os_handle_internal_module_error(current_req.request_number);
                merry_os_prepare_for_exit(); // now since this is an error, we can't continue
                break;
            }
            else if (_MERRY_REQUEST_PROGRAM_ERROR_(current_req.request_number))
            {
                os.err_core_id = current_req.id;
                merry_os_handle_error(current_req.request_number, current_req.id); // this will handle all errors
                merry_os_prepare_for_exit();
                break;
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
                    case _REQ_MEM:
                        merry_os_execute_request_mem(&os, &current_req);
                        break;
                    case _REQ_NEWPROCESS:
                        merry_os_execute_request_newprocess(&os, &current_req);
                        break;
                    case _REQ_INTR:
                        if (os.reader->de_flag == mfalse)
                            break; // ignore it
                        merry_os_execute_request_intr(&os, &current_req);
                        continue;
                    case _REQ_BP:
                        if (os.listener_running == mtrue && os.sender_running == mtrue)
                            merry_os_execute_request_bp(&os, &current_req);
                        break;
                    case _REQ_GDB_INIT:
                        if (current_req.id == 1)
                            os.sender_running = mtrue;
                        else
                            os.listener_running = mtrue;
                        break;
                    case _REQ_LOAD_LIB:
                        merry_os_execute_request_dynl(&os, &current_req);
                        break;
                    case _REQ_UNLOAD_LIB:
                        merry_os_execute_request_dynul(&os, &current_req);
                        break;
                    case _REQ_GET_FUNC:
                        merry_os_execute_request_dyncall(&os, &current_req);
                        break;
                    case _REQ_SYSCALL:
                        merry_os_execute_request_syscall(&os, &current_req);
                        break;
                    case _REQ_GET_FUNC_ADDR:
                        merry_os_execute_request_get_func_addr(&os, &current_req);
                        break;
                    case _REQ_CALL_LOADED_FUNC:
                        merry_os_execute_request_call_loaded_func(&os, &current_req);
                        break;
                    case _REQ_START_SUBSYS:
                        merry_os_execute_request_start_subsys(&os, &current_req);
                        break;
                    case _REQ_SUBSYS_ADD_CHANNEL:
                        merry_os_execute_request_add_channel(&os, &current_req);
                        break;
                    case _REQ_SUBSYS_CLOSE_CHANNEL:
                        merry_os_execute_request_close_channel(&os, &current_req);
                        break;
                    case _REQ_SUBSYS_SEND:
                        merry_os_execute_request_send(&os, &current_req);
                        break;
                    case _REQ_SUBSYS_SEND_WAIT:
                        merry_os_execute_request_send_wait(&os, &current_req);
                        continue; // the subsystem will wake up the core
                    case _REQ_GETERRNO:
                        merry_os_execute_request_geterrno(&os, &current_req);
                        break;
                    case _REQ_SUBSYS_STATUS:
                        merry_os_execute_request_subsys_status(&os, &current_req);
                        break;
                    default:
                        fprintf(stderr, "Error: Unknown request code: '%llu' is not a valid request code.\n", current_req.request_number);
                        break;
                    }
                }
            }
            // after the fulfillment of the request, wake up the core
            merry_cond_signal(current_req._wait_lock);
        }
    }
    if (os.ret == _MERRY_EXIT_SUCCESS_)
        merry_os_prepare_for_exit();
    // we won't wait at all now
    // while (os.listener_stopped != mtrue || os.sender_stopped != mtrue)
    // {
    // }
    // dump data if necessary
    if (os.ret == _MERRY_EXIT_FAILURE_ && os.dump_on_error == mtrue)
    {
        if (merry_os_error_dump() == RET_FAILURE)
        {
            rlog("Internal Error: Couldn't dump the internal state.\n", NULL);
        }
    }
#if defined(_MERRY_HOST_OS_LINUX_)
    return (mptr_t)(&os.ret); // freeing the OS is the Main's Job
#elif defined(_MERRY_HOST_OS_WINDOWS_)
    return os.ret;
#endif
}

void merry_os_handle_others(merrot_t _id, msize_t id)
{
    switch (_id)
    {
    case _SHOULD_EXIT:
        os.stop = mtrue;
        os.ret = _MERRY_EXIT_SUCCESS_;
        break;
    case MERRY_SEGV:
        merry_mem_error("Segmentation Fault(Passed by the Host system)");
        os.stop = mtrue;
        os.ret = _MERRY_EXIT_FAILURE_;
        break;
    case MERRY_SUBSYS_FAILED:
        os._subsystem_failure = mtrue;
        merry_subsys_close_all();
        break;
    case MERRY_SUBSYS_INIT_FAILURE:
        os._subsystem_failure = mtrue;
        break;
    }
}

void merry_os_handle_error(merrot_t error, msize_t id)
{
    // this sets the return value for us
    os.ret = _MERRY_EXIT_FAILURE_;
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
    case MERRY_INTERNAL_ERROR:
        merry_general_error("Internal Machine Error", "This isn't your fault most probably, try running the program again.");
        break;
    default:
        if (os.cores[id]->excp_set == mtrue)
        {
            os.cores[id]->pc = os.cores[id]->exception_address;
            return;
        }
        merry_error("Unknown error code: '%llu' is not a valid error code", error);
        break;
    }
    merry_os_notify_dbg(_ERROR_ENCOUNTERED_, error, os.err_core_id);
}

void merry_os_handle_internal_module_error(merrot_t error_num)
{
    os.ret = _MERRY_EXIT_FAILURE_;
    switch (error_num)
    {
    case _PANIC_REQBUFFEROVERFLOW:
        // either the program is deliberately trying to do this or the number is cores is just too much and requests to fast
        merry_internal_module_error("The request buffer hit maximum capacity. Cannot fulfill requests");
        break;
    }
    merry_os_notify_dbg(_ERROR_ENCOUNTERED_, error_num, os.err_core_id);
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
    merry_os_notify_dbg(_CORE_TERMINATING_, request->id, 0);
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
    os->cores[request->id]->registers[Mb] = errno;
    return RET_SUCCESS; // for now
}

_os_exec_(mem)
{
    // each time this request is made, a new page is allocated
    // the starting address of the new page will be returned in Mb register
    // if failed, Ma will contain 1 else 0
    if (merry_dmemory_add_new_page(os->data_mem) == RET_FAILURE)
    {
        os->cores[request->id]->registers[Ma] = 1;
        os->cores[request->id]->registers[Mb] = errno;
        return RET_FAILURE;
    }
    os->cores[request->id]->registers[Mb] = (os->data_mem->number_of_pages - 1) * _MERRY_MEMORY_ADDRESSES_PER_PAGE_ + 1;
    merry_os_notify_dbg(_ADDED_MEM_, 0, 0);
    return RET_SUCCESS;
}

_os_exec_(newprocess)
{
    // This will start a new process
    merry_requestHdlr_acquire();
    msize_t iport, oport;
    merry_os_get_io_port_direct(&iport, &oport);
    if ((iport == 0 || oport == 0) && os->reader->dfe_flag  == mtrue)
    {
        os->reader->dfe_flag = mfalse; // we won't take any more connections
        os->cores[request->id]->registers[Ma] = 1;
        merry_requestHdlr_release();
        return RET_FAILURE;
    }
#ifdef _USE_LINUX_
    MerryProcess p;
    if (merry_create_process(&p) == mfalse)
    {
        merry_requestHdlr_release();
        os->cores[request->id]->registers[Ma] = 1;
        os->cores[request->id]->registers[Mb] = errno;
        return RET_FAILURE;
    }
    if (p.pid == 0)
    {
        merry_os_set_env(iport, oport, request->id);
        msize_t argc;
        mstr_t *argv;
        merry_get_cmd_options(&argc, &argv);
        execv(/*Do something about this*/ "./build/mvm", argv);
        exit(EXIT_FAILURE);
    }
#elif _USE_WIN_
    merry_os_set_env(iport, oport, request->id);
    if (merry_create_process(&p) == mfalse)
    {
        merry_requestHdlr_release();
        os->cores[request->id]->registers[Ma] = 1;
        os->cores[request->id]->registers[Mb] = errno;
        return RET_FAILURE;
    }
#endif
    merry_requestHdlr_release();
    os->cores[request->id]->registers[Ma] = 0;
    merry_os_notify_dbg(_NEW_OS_, 0, 0);
    return RET_SUCCESS;
}

_os_exec_(intr)
{
    maddress_t addr;
    mqword_t x;
    mbyte_t reply[_MERRY_PER_EXCG_BUF_LEN_];
    reply[0] = _REPLY_;
    switch (request->opcode)
    {
    case _GET_CORE_COUNT_:
        reply[15] = os->core_count;
        break;
    case _GET_OS_ID_:
        reply[15] = os->_os_id;
        break;
    case _GET_DATA_MEM_PAGE_COUNT_:
        reply[15] = os->data_mem->number_of_pages;
        break;
    case _GET_INST_MEM_PAGE_COUNT_:
        reply[15] = os->inst_mem->number_of_pages;
        break;
    case _ADD_BREAKPOINT_:
        merry_memory_write(os->inst_mem, request->address, ((0xFFFFFFFFFFFFFFFF | OP_INTR) << 56) | _REQ_BP);
        break;
    case _INST_AT_:
        merry_memory_read(os->inst_mem, request->address, &x);
        merry_os_set_dbg_sig(x, reply);
        break;
    case _DATA_AT_:
        merry_dmemory_read_qword_atm(os->data_mem, request->address, &x);
        merry_os_set_dbg_sig(x, reply);
        break;
    case _SP_OF_:
        merry_os_set_dbg_sig(os->cores[request->arg_id & os->core_count]->sp, reply);
        break;
    case _BP_OF_:
        merry_os_set_dbg_sig(os->cores[request->arg_id & os->core_count]->bp, reply);
        break;
    case _PC_OF_:
        merry_os_set_dbg_sig(os->cores[request->arg_id & os->core_count]->pc, reply);
        break;
    case _REGR_OF_:
        merry_os_set_dbg_sig(os->cores[request->arg_id & os->core_count]->registers[request->address], reply);
        break;
    case _CONTINUE_CORE_:
        merry_cond_signal(os->cores[request->arg_id]->cond);
        break;
    }
    merry_sender_push_sig(os->sender, reply);
}

_os_exec_(bp)
{
    merry_os_notify_dbg(_HIT_BP_, request->id, 0);
}

_os_exec_(dynl)
{
    // the address to the name of the library must be in the Ma register
    // if the name is not and it is invalidly placed, the host will throw a segfault
    register MerryCore *c = os->cores[request->id];
    mstr_t name = merry_dmemory_get_bytes_maybe_over_multiple_pages_upto(os->data_mem, c->registers[Ma], 0);
    if (name == RET_NULL)
    {
        merry_requestHdlr_panic(os->data_mem->error, request->id);
        return RET_FAILURE;
    }
    if (merry_loader_loadLib(name, &c->registers[Mb]) == mfalse)
    {
        merry_set_errno(MERRY_DYNERR);
        c->registers[Ma] = 1;
        c->registers[Mb] = MERRY_DYNERR;
        free(name);
        return RET_FAILURE;
    }
    c->registers[Ma] = 0;
    free(name);
    return RET_SUCCESS;
}

_MERRY_ALWAYS_INLINE_ _os_exec_(dynul)
{
    merry_loader_unloadLib(os->cores[request->id]->registers[Ma]);
    return RET_SUCCESS;
}

_os_exec_(dyncall)
{
    dynfunc_t function;
    register MerryCore *c = os->cores[request->id];
    register msize_t len = c->registers[Md];
    mbptr_t param = merry_dmemory_get_bytes_maybe_over_multiple_pages(os->data_mem, c->registers[Mc], len);
    mbptr_t func_name = merry_dmemory_get_bytes_maybe_over_multiple_pages_upto(os->data_mem, c->registers[Mb], 0);
    if (param == NULL || func_name == NULL)
    {
        merry_set_errno(MERRY_DYNERR);
        c->registers[Mb] = MERRY_DYNERR;
        c->registers[Ma] = 1;
        if (!(func_name == NULL))
            free(func_name);
        return RET_FAILURE;
    }
    if ((function = merry_loader_getFuncSymbol(c->registers[Ma], func_name)) == RET_NULL)
    {
        merry_set_errno(MERRY_DYNERR);
        c->registers[Mb] = MERRY_DYNERR;
        c->registers[Ma] = 1;
        free(func_name);
        free(param);
        return RET_FAILURE;
    }
    os->cores[request->id]->registers[Ma] = function(param, len);
    free(func_name);
    free(param);
    return RET_SUCCESS;
}

_os_exec_(syscall)
{
    merry_exec_syscall(os->cores[request->id]);
}

_os_exec_(get_func_addr)
{
    dynfunc_t function;
    register MerryCore *c = os->cores[request->id];
    mbptr_t func_name = merry_dmemory_get_bytes_maybe_over_multiple_pages_upto(os->data_mem, c->registers[Mb], 0);
    if (func_name == NULL)
    {
        merry_set_errno(MERRY_DYNERR);
        c->registers[Mb] = MERRY_DYNERR;
        c->registers[Ma] = 1;
        if (!(func_name == NULL))
            free(func_name);
        return RET_FAILURE;
    }
    if ((function = merry_loader_getFuncSymbol(os->cores[request->id]->registers[Ma], func_name)) == RET_NULL)
    {
        merry_set_errno(MERRY_DYNERR);
        c->registers[Mb] = MERRY_DYNERR;
        c->registers[Ma] = 1;
        free(func_name);
        return RET_FAILURE;
    }
    MerryPtoQ p;
    p.ptr = (mptr_t)function;
    os->cores[request->id]->registers[Mb] = p.qword;
    c->registers[Ma] = 0;
    free(func_name);
    return RET_SUCCESS;
}

_os_exec_(call_loaded_func)
{
    register MerryCore *c = os->cores[request->id];
    dynfunc_t function = (dynfunc_t)c->registers[Mb];
    register msize_t len = c->registers[Md];
    mbptr_t param = merry_dmemory_get_bytes_maybe_over_multiple_pages(os->data_mem, c->registers[Mc], len);
    if (param == NULL)
    {
        merry_set_errno(MERRY_DYNERR);
        c->registers[Mb] = MERRY_DYNERR;
        c->registers[Ma] = 1;
        return RET_FAILURE;
    }
    if (merry_loader_is_still_valid(c->registers[Ma]) == mfalse)
    {
        merry_set_errno(MERRY_DYNCLOSED);
        c->registers[Mb] = MERRY_DYNCLOSED;
        c->registers[Ma] = 1;
        free(param);
    }
    os->cores[request->id]->registers[Ma] = function(param, len);
    free(param);
    return RET_SUCCESS;
}

_os_exec_(start_subsys)
{
    if (os->_subsystem_running == mtrue && os->_subsystem_failure == mfalse)
    {
        os->cores[request->id]->registers[Ma] = 0;
        return RET_SUCCESS;
    }
    os->cores[request->id]->registers[Ma] = merry_os_start_subsys();
    return RET_SUCCESS;
}

_os_exec_(add_channel)
{
    if (os->_subsystem_running == mfalse && os->_subsystem_failure == mfalse)
    {
        os->cores[request->id]->registers[Ma] = -1;
        return RET_SUCCESS;
    }
    msize_t id = merry_subsys_add_channel();
    register MerryCore *c = os->cores[request->id];
    // Ma = The address to a null-terminated string of the subsystem's name
    mstr_t name = merry_dmemory_get_bytes_maybe_over_multiple_pages_upto(os->data_mem, c->registers[Ma], 0);
    if (name == RET_NULL)
        goto err;
    if (id == (mqword_t)-1)
    {
        free(name);
        goto err;
    }
    MerrySubChannel *channel = merry_subsys_get_channel(id);
#ifdef _USE_LINUX_
    MerryProcess p;
    if (merry_create_process(&p) == mfalse)
    {
        c->registers[Ma] = -1;
        c->registers[Mb] = errno;
        return RET_FAILURE;
    }
    if (p.pid == 0)
    {
        char rfd[10];
        char wfd[10];
        snprintf(rfd, sizeof(rfd), "%d", channel->send_pipe->_read_fd);
        snprintf(wfd, sizeof(wfd), "%d", channel->receive_pipe->_write_fd);
        msize_t argc = 4;
        mstr_t const argv[] = {"./subsysmain", rfd, wfd, name, NULL};
        execv(/*Do something about this*/ "./subsysmain", argv);
        c->registers[Ma] = -1; // we failed
    }
#elif _USE_WIN_
    /// TODO: Update this to work on windows too.
    if (merry_create_process(&p) == mfalse)
    {
        c->registers[Ma] = -1;
        c->registers[Mb] = errno;
        return RET_FAILURE;
    }
#endif
    merry_config_channel(channel);
    merry_os_notify_subsys();
    c->registers[Ma] = id;
    return RET_SUCCESS;
err:
    c->registers[Ma] = -1;
    return RET_FAILURE;
}

_os_exec_(close_channel)
{
    register MerryCore *c = os->cores[request->id];
    if (os->_subsystem_running == mfalse && os->_subsystem_failure == mfalse)
    {
        c->registers[Ma] = 1;
        return RET_FAILURE;
    }
    merry_subsys_close_channel(c->registers[Ma]);
    c->registers[Ma] = 0;
    return RET_FAILURE;
}

_os_exec_(send)
{
    register MerryCore *c = os->cores[request->id];
    if (os->_subsystem_running == mfalse && os->_subsystem_failure == mfalse)
    {
        c->registers[Ma] = 1;
        return RET_FAILURE;
    }
    merry_subsys_write(c->registers[Ma], c->registers[Mb], c->registers[M1], c->registers[M2], c->registers[M3], c->registers[M4]);
    c->registers[Ma] = 0;
    return RET_SUCCESS;
}

_os_exec_(send_wait)
{
    register MerryCore *c = os->cores[request->id];
    if (os->_subsystem_running == mfalse && os->_subsystem_failure == mfalse)
    {
        c->registers[Ma] = 1;
        return RET_FAILURE;
    }
    merry_subsys_write(c->registers[Ma], c->registers[Mb], c->registers[M1], c->registers[M2], c->registers[M3], c->registers[M4]);
    merry_subsys_add_task(c->registers[Ma], c->cond, &c->registers[Ma]);
    return RET_SUCCESS;
}

_os_exec_(geterrno)
{
    os->cores[request->id]->registers[Ma] = errno;
    os->cores[request->id]->registers[Mb] = merry_get_errno();
}

_os_exec_(subsys_status)
{
    os->cores[request->id]->registers[Ma] = os->_subsystem_running == mtrue ? 0 : 1;
    os->cores[request->id]->registers[Mb] = os->_subsystem_failure == mtrue ? 0 : 1;
}

void merry_os_notify_dbg(mqword_t sig, mbyte_t arg, mbyte_t arg2)
{
    if (os.listener_running != mtrue || os.sender_running != mtrue)
        return;
    mbyte_t _to_send[_MERRY_PER_EXCG_BUF_LEN_];
    _to_send[0] = sig;
    switch (sig)
    {
    case _NEW_CORE_:
    case _TERMINATING_:
    case _HIT_BP_:
    case _CORE_TERMINATING_:
        _to_send[15] = arg;
        break;
    case _ERROR_ENCOUNTERED_:
        _to_send[15] = arg;
        _to_send[14] = arg2;
        break;
    case _NEW_OS_:
    case _ADDED_MEM_:
        break;
    }
    merry_sender_push_sig(os.sender, _to_send);
}

void merry_os_get_io_port_direct(msize_t *ip, msize_t *op)
{
    if ((os.listener_running != mtrue || os.sender_running != mtrue))
    {
        *ip = *op = 0;
        if (os.reader->dfe_flag != mfalse)
            merry_set_errno(MERRY_NODBG);
        return;
    }
    mbyte_t _to_send[_MERRY_PER_EXCG_BUF_LEN_];
    _to_send[0] = _NEW_PROC_IO_PORT_NUM_;
    merry_sender_push_sig(os.sender, _to_send);
    while (os.listener->t1 == 0 && os.listener->t2 == 0)
    {
    }
    *ip = os.listener->t1;
    *op = os.listener->t2;
    os.listener->t1 = os.listener->t2 = 0;
}

/*UN-USED AND DEPRECATED*/
void merry_os_new_proc_init(msize_t ip, msize_t op)
{
    // Here, we reinitialize everything from scratch.
    // In case of Linux, the OS thread is replicated in the new process
    // We need to free everything accept the OS and the reader and then redo the initialization
    // I do believe that mutexes and condition variables are different for different processes unless provided with attributes
    fflush(stdin);
    merry_requestHdlr_release();
    os.reader->eat.eat_entry_count = 1;
    if (merry_os_init_reader_provided(os.reader, ip, op) == RET_FAILURE)
    {
        rlog("Internal Error: Failed to start new process.\n", NULL);
        exit(EXIT_FAILURE);
    }
}

void merry_os_produce_dump(mstr_t _output_filename)
{
    // The OS should produce output dump file
    os.dump_on_error = mtrue;
    os.dump_file = _output_filename; // _output_filename is an allocated string
}

void merry_os_dump_mem_dets(FILE *f)
{
    fprintf(f, "MEMORY INFO[DATA MEMORY]\n", os._os_id);
    fprintf(f, "\tNumber of pages: %lu\n", os.data_mem->number_of_pages);
    fprintf(f, "NOTE: Size of page: 1MB\n\n");
    fprintf(f, "MEMORY INFO[INSTRUCTION MEMORY]\n", os._os_id);
    fprintf(f, "\tNumber of pages: %lu\n", os.inst_mem->number_of_pages);
    fprintf(f, "NOTE: Size of page: 1MB\n\n");
}

void merry_os_dump_core_dets(FILE *f)
{
    fprintf(f, "CORE DETAILS\n");
    for (msize_t i = 0; i < os.core_count; i++)
    {
        // os.cores[i].
        MerryCore *c = os.cores[i];
        mbptr_t sym = NULL;
        fprintf(f, "\nVIRTUAL CORE[ID:%lu]%s\n", i, (i == os.err_core_id) ? "(CAUSE OF TERMINATION)" : "");
        fprintf(f, "\tBP(At the time of termination):%lX\n", c->bp);
        fprintf(f, "\tSP(At the time of termination):%lX\n", c->sp);
        fprintf(f, "\tPC(At the time of termination):%lX\n", c->pc);
        fprintf(f, "\tIR(At the time of termination):%lX\n", c->current_inst);
        fprintf(f, "\tSTACK SIZE:Static Stack(Upwards):1MB\n");
        fprintf(f, "\tException Address(Set by the program):%lX(%s)\n", c->exception_address, c->excp_set == mtrue ? (sym = merry_reader_get_symbol(os.reader, c->exception_address)) == NULL ? "NO SYMBOL FOUND" : (mstr_t)sym : "EXCP NOT SET");
        fprintf(f, "\tFLAGS REGISTER:\n");
        fprintf(f, "\t\tCARRY    :%lu\n", c->flag.carry);
        fprintf(f, "\t\tZERO     :%lu\n", c->flag.zero);
        fprintf(f, "\t\tNEGATIVE :%lu\n", c->flag.negative);
        fprintf(f, "\t\tOVERFLOW :%lu\n", c->flag.overflow);
        fprintf(f, "\t\tGREATER  :%lu\n", c->greater);
        fprintf(f, "\tREGITSER FILE:\n");
        fprintf(f, "\t\tMa :%lu\n", c->registers[Ma]);
        fprintf(f, "\t\tMb :%lu\n", c->registers[Mb]);
        fprintf(f, "\t\tMc :%lu\n", c->registers[Mc]);
        fprintf(f, "\t\tMd :%lu\n", c->registers[Md]);
        fprintf(f, "\t\tMe :%lu\n", c->registers[Me]);
        fprintf(f, "\t\tMf :%lu\n", c->registers[Mf]);
        fprintf(f, "\t\tM1 :%lu\n", c->registers[M1]);
        fprintf(f, "\t\tM2 :%lu\n", c->registers[M2]);
        fprintf(f, "\t\tM3 :%lu\n", c->registers[M3]);
        fprintf(f, "\t\tM4 :%lu\n", c->registers[M4]);
        fprintf(f, "\t\tM5 :%lu\n", c->registers[M5]);
        fprintf(f, "\t\tMm1:%lu\n", c->registers[Mm1]);
        fprintf(f, "\t\tMm2:%lu\n", c->registers[Mm2]);
        fprintf(f, "\t\tMm3:%lu\n", c->registers[Mm3]);
        fprintf(f, "\t\tMm4:%lu\n", c->registers[Mm4]);
        fprintf(f, "\t\tMm5:%lu\n", c->registers[Mm5]);
        fprintf(f, "\tCALL TRACE:\n");
        if (stack_empty(c))
            continue;
        mqword_t addr = 0;
        while (merry_stack_pop(c->callstack, &addr) != RET_FAILURE)
        {
            fprintf(f, "\t\t%lX(%s)\n", addr, (sym = merry_reader_get_symbol(os.reader, addr)) == RET_NULL ? "NO SYMBOL FOUND" : (mstr_t)sym);
        }
        fprintf(f, "\t\t%lX(%s)\n", c->entry_addr, (sym = merry_reader_get_symbol(os.reader, c->entry_addr)) == RET_NULL ? "NO SYMBOL FOUND" : (mstr_t)sym);
    }
}

mret_t merry_os_error_dump()
{
    // We first put into the necessary details one by one
    // First a file
    char finalf[strlen(os.dump_file == NULL ? _MERRY_DEFAULT_DUMP_FILE_ : os.dump_file) + 2];
    char x[2] = {(char)(48 + os._os_id), '\0'};
    strcpy(finalf, os.dump_file == NULL ? _MERRY_DEFAULT_DUMP_FILE_ : os.dump_file);
    strcat(finalf, x);
    FILE *df = fopen(finalf, "w");
    if (df == NULL)
        return RET_FAILURE;
    // The details about the OS
    fprintf(df, "MANAGING THREAD[PROCESS %lu]\n", os._os_id);
    fprintf(df, "\tCore count(At the time of termination): %lu\n", os.core_count);
    fprintf(df, "\tCause of Termination: CORE ID=%lu\n\n", os.err_core_id);
    merry_os_dump_mem_dets(df);
    merry_os_dump_core_dets(df);
    fclose(df);
    return RET_SUCCESS;
}

mqword_t merry_os_get_dbg_sig(mbptr_t sig)
{
    mqword_t x = sig[8];
    (x <<= 8);
    x |= sig[9];
    (x <<= 8);
    x |= sig[10];
    (x <<= 8);
    x |= sig[11];
    (x <<= 8);
    x |= sig[12];
    (x <<= 8);
    x |= sig[13];
    (x <<= 8);
    x |= sig[14];
    (x <<= 8);
    x |= sig[15];
    return x;
}

void merry_os_set_dbg_sig(mqword_t _sig, mbptr_t sig)
{
    sig[8] = _sig >> 56;
    sig[9] = (_sig >> 48) & 255;
    sig[10] = (_sig >> 40) & 255;
    sig[11] = (_sig >> 32) & 255;
    sig[12] = (_sig >> 24) & 255;
    sig[13] = (_sig >> 16) & 255;
    sig[14] = (_sig >> 8) & 255;
    sig[15] = (_sig) & 255;
}

void merry_os_notice(mbool_t _type)
{
    // printf("DID: %lu\n", _type);
    if (_type == mtrue)
        atomic_exchange(&os.listener_running, mtrue);
    else if (_type == mfalse)
        atomic_exchange(&os.sender_running, mtrue);
    else if (_type == 2)
        atomic_exchange(&os.sender_stopped, mtrue);
    else
        atomic_exchange(&os.listener_stopped, mtrue);
}

mqword_t merry_os_get_ret()
{
    return os.ret;
}

void merry_os_set_env(msize_t ip, msize_t op, msize_t id)
{
#ifdef _USE_WIN_
    char tmp[16];
    sprintf(tmp, "%d", ip);
    SetEnvironmentVariable("_MERRY_CHILD_DEBUG_", os->reader->dfe_flag == mtrue ? "yes" : "no");
    SetEnvironmentVariable("_MERRY_CHILD_FREEZE_", os->reader->dfw_flag == mtrue ? "yes" : "no");
    SetEnvironmentVariable("_MERRY_IPORT_", tmp);
    sprintf(tmp, "%d", op);
    SetEnvironmentVariable("_MERRY_OPORT_", tmp);
    sprintf(tmp, "%llu", os.cores[id]->entry_addr);
    SetEnvironmentVariable("_MERRY_ADDR_", tmp);
    SetEnvironmentVariable("_MERRY_CHILD_SURVEY_", "yes");
#elif defined(_USE_LINUX_)
    char tmp[17];
    sprintf(tmp, "%lX", ip);
    setenv("_MERRY_CHILD_DEBUG_", os.reader->dfe_flag == mtrue ? "yes" : "no", 1);
    setenv("_MERRY_CHILD_FREEZE_", os.reader->dfw_flag == mtrue ? "yes" : "no", 1);
    setenv("_MERRY_IPORT_", tmp, 1);
    sprintf(tmp, "%lX", op);
    setenv("_MERRY_OPORT_", tmp, 1);
    sprintf(tmp, "%lX", os.cores[id]->entry_addr);
    setenv("_MERRY_ADDR_", tmp, 1);
    setenv("_MERRY_CHILD_SURVEY_", "yes", 1);
#endif
}