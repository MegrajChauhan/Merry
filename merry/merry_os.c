#include "merry_os.h"

mret_t merry_os_init(mcstr_t _inp_file, char **options, msize_t count, mbool_t _wait_for_conn)
{
    // initialize the os
    os._os_id = 0;
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
    /// NOTE: Don't mind the nested if-conditions if you will.
    if (os.reader->de_flag == mtrue)
    {
        // Initialize debugger threads
        os.listener = merry_init_listener(_wait_for_conn);
        if (os.listener != RET_NULL)
        {
            os.sender = merry_init_sender(_wait_for_conn);
            if (os.sender != RET_NULL)
            {
                os.listener_th = merry_thread_init();
                os.sender_th = merry_thread_init();
                if (os.listener_th == NULL || os.sender_th == NULL)
                {
                    merry_destroy_reader(os.sender);
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
            while (_wait_for_conn == mtrue && os.listener_running != mtrue && os.sender_running != mtrue)
            {
            }
        }
        else
            rlog("Internal Error: DE flag provided but couldn't start the debugger threads.\n", NULL);
    }
    os.cores[0]->registers[Mm1] = count;        // Mm1 will have the number of options
    os.cores[0]->registers[Md] = _t + 1;        // Md will have the address to the first byte
    os.cores[0]->registers[Mm5] = len + _t + 1; // Mm5 contains the address of the first byte that is free and can be manipulated by the program
    os.cores[0]->registers[Mm4] = _MERRY_HOST_ID_ARCH_;
    os.cores[0]->registers[Mm3] = _MERRY_HOST_ID_OS_;
    os.stop = mfalse;
    os.core_threads = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count));
    if (os.core_threads == RET_NULL)
        goto failure;
    os.active_core_count = 0;
    os.err_core_id = 0;
    os.ret = _MERRY_EXIT_SUCCESS_;
    return RET_SUCCESS; // we did everything correctly
failure:
    merry_os_destroy();
    return RET_FAILURE;
}

mret_t merry_os_init_reader_provided(MerryReader *r)
{
    // we need to put the options into the memory
    os._os_id = os._os_id + 1;
    os.reader = r;
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
    // We need to define some convention that will be useful for multi-processing
    // if (os.reader->de_flag == mtrue)
    // {
    //     // Once we see that we have this flag
    //     // we first prepare for connection from the debugger
    //     os.dbg = merry_init_dbsupp();
    //     if (os.dbg == RET_NULL)
    //         rlog("Internal Error: Failed to initialize connection to debugger.\n", NULL);
    //     else
    //     {
    //         os.dbg_th = merry_thread_init();
    //         if (os.dbg_th == RET_NULL)
    //             rlog("Internal Error: Could not connect to the debugger.\n", NULL);
    //         else
    //         {
    //             if (merry_create_detached_thread(os.dbg_th, &merry_dbg_run, os.dbg) == RET_FAILURE)
    //                 os.dbg_running = mfalse;
    //             else
    //                 os.dbg_running = mtrue;
    //         }
    //     }
    // }
    os.stop = mfalse;
    os.core_threads = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count));
    if (os.core_threads == RET_NULL)
        goto failure;
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
    merry_destroy_listener(os.listener);
    merry_destroy_sender(os.sender);
    merry_thread_destroy(os.listener_th);
    merry_thread_destroy(os.sender_th);
    merry_destroy_reader(os.reader);
    merry_requestHdlr_destroy();
}

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
    merry_os_notify_dbg(_NEW_CORE_, os.core_count - 1, 0);
    return RET_SUCCESS;
}

mret_t merry_os_add_core()
{
    // just add another core
    merry_mutex_lock(os._lock);
    MerryCore *new_core = merry_core_init(os.inst_mem, os.data_mem, os.core_count);
    if (new_core == RET_NULL)
    {
        merry_mutex_unlock(os._lock);
        return RET_FAILURE;
    }
    MerryThread *th = merry_thread_init();
    if (th == RET_NULL)
    {
        merry_core_destroy(new_core, mtrue);
        merry_mutex_unlock(os._lock);
        return RET_FAILURE;
    }
    MerryThread **temp = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count));
    if (temp == NULL)
    {
        merry_core_destroy(new_core, mtrue);
        merry_thread_destroy(th);
        merry_mutex_unlock(os._lock);
        return RET_FAILURE; // we failed
    }
    MerryCore **tempc = (MerryCore **)malloc(sizeof(MerryCore *) * (os.core_count));
    if (tempc == NULL)
    {
        // we failed again
        merry_core_destroy(new_core, mtrue);
        merry_thread_destroy(th);
        merry_mutex_unlock(os._lock);
        free(temp);
        return RET_FAILURE;
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
    merry_os_notify_dbg(_TERMINATING_, os._os_id, 0);
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
            if (_MERRY_REQUEST_INTERNAL_ERROR_(current_req.request_number))
            {
                os.err_core_id = current_req.id;
                merry_os_handle_internal_module_error(current_req.request_number);
                merry_os_prepare_for_exit(); // now since this is an error, we can't continue
                break;
            }
            else if (_MERRY_REQUEST_PROGRAM_ERROR_(current_req.request_number))
            {
                os.err_core_id = current_req.id;
                merry_os_handle_error(current_req.request_number); // this will handle all errors
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
                        break;
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

void merry_os_handle_error(merrot_t error)
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
    case MERRY_INTERNAL_ERROR:
        merry_general_error("Internal Machine Error", "This isn't your fault most probably, try running the program again.");
        break;
    default:
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
    os->reader->eat.EAT[0] = os->cores[request->id]->pc; // we don't mind this to change here
    fflush(stdout);
    fflush(stderr);
#ifdef _USE_LINUX_
    MerryProcess p;
    if (merry_create_process(&p) == mfalse)
    {
        merry_requestHdlr_release();
        os->cores[request->id]->registers[Ma] = 1;
        return RET_FAILURE;
    }
    if (p.pid == 0)
    {
        // We are in the child process
        merry_os_new_proc_init();
        merry_os_start_vm(NULL);
        msize_t r = os->ret;
        merry_os_destroy();
        exit(r);
    }
#endif
    merry_requestHdlr_release();
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
}

_os_exec_(bp)
{
    merry_os_notify_dbg(_HIT_BP_, request->id, 0);
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

void merry_os_new_proc_init()
{
    // Here, we reinitialize everything from scratch.
    // In case of Linux, the OS thread is replicated in the new process
    // We need to free everything accept the OS and the reader and then redo the initialization
    // I do believe that mutexes and condition variables are different for different processes unless provided with attributes
    fflush(stdin);
    merry_requestHdlr_release();
    merry_os_new_proc_cleanup();
    os.reader->eat.eat_entry_count = 1;
    if (merry_os_init_reader_provided(os.reader) == RET_FAILURE)
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
        continue;
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
        while (merry_stack_pop(c->ras, &addr) != RET_FAILURE)
        {
            fprintf(f, "\t\t%lX(%s)\n", addr, (sym = merry_reader_get_symbol(os.reader, addr)) == NULL ? "NO SYMBOL FOUND" : (mstr_t)sym);
        }
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
    if (_type == mtrue)
        atomic_store(&os.listener_running, mtrue);
    else
        atomic_store(&os.sender_running, mtrue);
}