#include "merry_os.h"

mret_t merry_os_init(mcstr_t _inp_file, char **options, msize_t count, mbool_t _wait_for_conn)
{
    inlog("Initializing the MANAGER THREAD");
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
    inlog("Initializing Data Memory");
    if ((os.data_mem = merry_dmemory_init_provided(os.reader->data, os.reader->data_page_count)) == RET_NULL)
    {
        merry_destroy_reader(os.reader);
        return RET_FAILURE;
    }

    // we need to put the options into the memory
    inlog("Putting the Options in their place....");
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
    inlog("Initializing Instruction Memory");
    // perform initialization for the inst mem as well. Unlike data memory, instruction page len cannot be 0
    if ((os.inst_mem = merry_memory_init_provided(os.reader->inst.instructions, os.reader->inst.inst_page_count)) == RET_NULL)
        goto failure;
    // initialize all the cores
    os.cores = (MerryCore **)malloc(sizeof(MerryCore *) * os.reader->eat.eat_entry_count);
    if (os.cores == NULL)
    {
        mreport("Failed to allocate memory for the VCores");
        goto failure;
    }
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
    /// NOTE: Don't mind the nested if-conditions if you will.
    os.wait_for_conn = _wait_for_conn;
    os.dbg_running = mfalse;
    if (os.reader->de_flag == mtrue)
        merry_os_start_dbg();
    os.cores[0]->registers[Mm1] = count;    // Mm1 will have the number of options
    os.cores[0]->registers[Md] = _t;        // Md will have the address to the first byte
    os.cores[0]->registers[Mm5] = len + _t; // Mm5 contains the address of the first byte that is free and can be manipulated by the program
    os.cores[0]->registers[Mm4] = _MERRY_HOST_ID_ARCH_;
    os.cores[0]->registers[Mm3] = _MERRY_HOST_ID_OS_;
    os.cores[0]->registers[Mm2] = os.data_mem->number_of_pages; // Number of pages that the program can manipulate
    os.stop = mfalse;
    os.core_threads = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count));
    if (os.core_threads == RET_NULL)
    {
        mreport("Failed to allocate memory for the VCores");
        goto failure;
    }
    os.active_core_count = 0;
    os.err_core_id = 0;
    os.ret = _MERRY_EXIT_SUCCESS_;
    os._subsystem_running = mfalse;
    os._subsystem_failure = mfalse;
    inlog("Installing Traps....");
    merry_trap_install(); // for now, the failure of this doesn't matter
    return RET_SUCCESS;   // we did everything correctly
failure:
    merry_os_destroy();
    return RET_FAILURE;
}

mret_t merry_os_start_subsys()
{
    inlog("Starting SubSystem...");
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

mret_t merry_os_init_reader_provided(MerryReader *r)
{
    // we need to put the options into the memory
    inlog("Initializing the MANAGER THREAD");
    os.reader = r;
    inlog("Initializing Data Memory");
    if ((os.data_mem = merry_dmemory_init_provided(os.reader->data, os.reader->data_page_count)) == RET_NULL)
    {
        merry_destroy_reader(os.reader);
        return RET_FAILURE;
    }
    inlog("Initializing Instruction Memory");
    if ((os.inst_mem = merry_memory_init_provided(os.reader->inst.instructions, os.reader->inst.inst_page_count)) == RET_NULL)
        goto failure;
    if (merry_loader_init(_MERRY_INITIAL_DYNLOAD_COUNT_) == mfalse)
        return RET_FAILURE;
    msize_t _t = os.reader->data_len;
    // initialize all the cores
    os.cores = (MerryCore **)malloc(sizeof(MerryCore *) * os.reader->eat.eat_entry_count);
    if (os.cores == NULL)
    {
        mreport("Failed to allocate memory for the VCores");
        goto failure;
    }
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
    os.wait_for_conn = os.reader->dfw_flag;
    os.dbg_running = mfalse;
    if (os.reader->dfe_flag == mtrue)
        merry_os_start_dbg();
    os.stop = mfalse;
    os.core_threads = (MerryThread **)malloc(sizeof(MerryThread *) * (os.core_count));
    if (os.core_threads == RET_NULL)
        goto failure;
    os.active_core_count = 0;
    // This init is done with a child process
    // let them know that they are a child process
    os.cores[0]->registers[Ma] = 1;
    inlog("Installing Traps....");
    merry_trap_install();
    return RET_SUCCESS; // we did everything correctly
failure:
    merry_os_destroy();
    return RET_FAILURE;
}

void merry_os_start_dbg()
{
    inlog("Starting Debugging...");
    os.dbg_th = merry_thread_init();
    if (os.dbg_th == RET_NULL)
    {
        mreport("Failed to start the debugger");
        os.reader->dfw_flag = mfalse;
        os.wait_for_conn = mfalse;
        return;
    }
    if (((os.dbg = merry_init_debug(os._os_id)) == RET_NULL) || (merry_create_detached_thread(os.dbg_th, &merry_start_debugging, os.dbg) == RET_FAILURE))
    {
        mreport("Debugger couldn't initialize");
        os.reader->dfw_flag = mfalse;
        os.wait_for_conn = mfalse;
        return;
    }
    os.dbg_running = mtrue;
    return;
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
    merry_destroy_debug(os.dbg);
    merry_thread_destroy(os.dbg_th);
    merry_destroy_reader(os.reader);
    merry_loader_close();
    merry_requestHdlr_destroy();
}

mret_t merry_os_boot_core(msize_t core_id, maddress_t start_addr)
{
    // this function's job is to boot up the core_id core and prepare it for execution
    os.cores[core_id]->pc = start_addr; // point to the starting address of the core
    os.cores[core_id]->entry_addr = start_addr;
    // now start the core thread
    log("Booting VCore %lu", core_id);
    if ((os.core_threads[core_id] = merry_thread_init()) == RET_NULL)
    {
        return RET_FAILURE;
    }
    if (merry_create_detached_thread(os.core_threads[core_id], &merry_runCore, os.cores[core_id]) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    os.active_core_count++;
    merry_os_notify_dbg(_NEW_CORE_, core_id);
    return RET_SUCCESS;
}

mret_t merry_os_add_core()
{
    // just add another core
    inlog("Adding a new core...");
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
        mreport("Failed to allocate memory for a new core");
        merry_core_destroy(new_core, mtrue);
        merry_thread_destroy(th);
        goto _err;
    }
    MerryCore **tempc = (MerryCore **)malloc(sizeof(MerryCore *) * (os.core_count));
    if (tempc == NULL)
    {
        // we failed again
        mreport("Failed to allocate memory for a new core");
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

_MERRY_INTERNAL_ void merry_os_notify_subsys(mstr_t buf, msize_t len)
{
    ssize_t r = write(os.os_pipe->_write_fd, buf, len); // wake it up
}

_MERRY_INTERNAL_ void merry_os_prepare_for_exit()
{
    // prepare for termination
    // firstly tell all cores to shut down
    inlog("Preparing for Termination...");
    for (msize_t i = 0; i < os.core_count; i++)
    {
        atomic_exchange(&os.cores[i]->stop_running, mtrue);
    }
    // some cores may be waiting for their requests to be fulfilled
    merry_requestHdlr_kill_requests(); // kill all requests
    merry_os_notify_dbg(_TERMINATING_, 0);
    if (os.dbg_running == mtrue)
    {
        os.dbg_running = mfalse;
        merry_os_notify_dbg(_CLOSE_, 0);
    }
    if ((os._subsystem_running == mtrue) && (os._subsystem_failure == mfalse))
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
    inlog("Manager is starting now....");
    MerryOSRequest current_req;
    while (os.wait_for_conn == mtrue)
    {
        if (merry_requestHdlr_pop_request(&current_req) == mfalse)
        {
            merry_cond_wait(os._cond, os._lock);
        }
        else
        {
            if (current_req.request_number == _DBG_ACTIVE_)
                break;
            else if (current_req.request_number == _SHOULD_EXIT || current_req.request_number == MERRY_SEGV)
                return (mptr_t)&os.ret;
            // ignore every other request right now(This isn't the way to go and any core isn't running either so it doesn't matter)
        }
    }
    inlog("Booting up VCores...");
    for (msize_t i = 0; i < os.core_count; i++)
    {
        if (merry_os_boot_core(i, os.reader->eat.EAT[i]) != RET_SUCCESS)
        {
            merry_os_prepare_for_exit();
            return (mptr_t)RET_FAILURE;
        }
    }
    mptr_t temp;
    while (mtrue)
    {
        if (os.active_core_count == 0)
            break;
        if (merry_requestHdlr_pop_request(&current_req) == mfalse)
        {
            // we have no requests to fulfill and so we goto sleep and wait for the request handler to wake us up
            if (os.stop == mtrue)
                break;
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
                break;
            }
            else if (_MERRY_REQUEST_PROGRAM_ERROR_(current_req.request_number))
            {
                os.err_core_id = current_req.id;
                merry_os_handle_error(current_req.request_number, current_req.id); // this will handle all errors
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
                    case _REQ_BP:
                        if (os.dbg_running == mtrue)
                        {
                            merry_os_execute_request_bp(&os, &current_req);
                            continue; // the debugger needs to continue this core
                        }
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
                        if (merry_os_execute_request_send_wait(&os, &current_req) == RET_FAILURE)
                            break;
                        continue; // the subsystem will wake up the core
                    case _REQ_GETERRNO:
                        merry_os_execute_request_geterrno(&os, &current_req);
                        break;
                    case _REQ_SUBSYS_STATUS:
                        merry_os_execute_request_subsys_status(&os, &current_req);
                        break;
                    case _DBG_ACTIVE_:
                        break;
                    case _GET_CORE_COUNT_:
                        merry_os_execute_request_get_core_count(&os, &current_req);
                        break;
                    case _GET_ACTIVE_CORE_COUNT_:
                        merry_os_execute_request_get_active_core_count(&os, &current_req);
                        break;
                    case _GET_DATA_MEM_PAGE_COUNT_:
                        merry_os_execute_request_get_data_mem_page_count(&os, &current_req);
                        break;
                    case _GET_INST_MEM_PAGE_COUNT_:
                        merry_os_execute_request_get_inst_mem_page_count(&os, &current_req);
                        break;
                    case _ADD_BREAKPOINT_:
                        merry_os_execute_request_add_breakpoint(&os, &current_req);
                        break;
                    case _INST_AT_:
                        merry_os_execute_request_inst_at(&os, &current_req);
                        break;
                    case _DATA_AT_:
                        merry_os_execute_request_data_at(&os, &current_req);
                        break;
                    case _SP_OF_:
                        merry_os_execute_request_sp_of(&os, &current_req);
                        break;
                    case _BP_OF_:
                        merry_os_execute_request_bp_of(&os, &current_req);
                        break;
                    case _PC_OF_:
                        merry_os_execute_request_pc_of(&os, &current_req);
                        break;
                    case _REGR_OF_:
                        merry_os_execute_request_regr_of(&os, &current_req);
                        break;
                    case _CONTINUE_CORE_:
                        merry_os_execute_request_continue_core(&os, &current_req);
                        break;
                    case _BT_OF_:
                        merry_os_execute_request_bt_of(&os, &current_req);
                        break;
                    case READ_DATA_PAGE:
                        merry_os_execute_request_read_data_pg(&os, &current_req);
                        break;
                    case READ_INST_PAGE:
                        merry_os_execute_request_read_inst_pg(&os, &current_req);
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
    merry_os_prepare_for_exit();
    while (os._subsystem_running == mtrue && os.dbg->stop == mfalse)
    {
    }
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

void merry_os_subsys_stopped()
{
    atomic_exchange(&os._subsystem_running, mfalse);
}

void merry_os_dbg_stopped()
{
    atomic_exchange(&os.dbg_running, mfalse);
}

void merry_os_handle_others(merrot_t _id, msize_t id)
{
    os.stop = mtrue;
    os.ret = _MERRY_EXIT_FAILURE_;
    switch (_id)
    {
    case _SHOULD_EXIT:
        os.ret = _MERRY_EXIT_SUCCESS_;
        break;
    case MERRY_SEGV:
        merry_mem_error("Segmentation Fault(Passed by the Host system)");
        break;
    case MERRY_SUBSYS_FAILED:
        merry_subsys_close_all();
        merry_error("Subsystem failed to continue(Speculative Termination)", NULL);
    case MERRY_SUBSYS_INIT_FAILURE:
        os._subsystem_failure = mtrue;
        merry_error("Subsystem failed to initialize", NULL);
        break;
    case MERRY_DBG_UNAVILABLE:
        merry_error("Debugger not available[Probably failed to start on VM end]", NULL);
        os.dbg_running = mfalse;
        os.dbg->stop = mtrue;
        os.stop = mfalse;
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
            os.cores[id]->pc = os.cores[id]->exception_address; // This doesn't save the return address and that is because we don't expect
                                                                // this to return
            return;
        }
        merry_error("Unknown error code: '%llu' is not a valid error code", error);
        break;
    }
    merry_os_notify_dbg(_ERROR_ENCOUNTERED_, (error << 8) | os.err_core_id);
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
    merry_os_notify_dbg(_ERROR_VM_, error_num);
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
    merry_os_notify_dbg(_CORE_TERMINATING_, request->id);
    return RET_SUCCESS;
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
    merry_os_notify_dbg(_ADDED_MEM_, 0);
    return RET_SUCCESS;
}

_os_exec_(newprocess)
{
    // This will start a new process
    register MerryCore *c = os->cores[request->id];
    register mqword_t _len = c->registers[Mb];
    mstr_t *_args = (mstr_t *)merry_dmemory_get_bytes_maybe_over_multiple_pages(os->data_mem, c->registers[Ma], _len);
    if (_args == RET_NULL)
    {
        c->registers[Ma] = 1;
        return RET_FAILURE;
    }
    merry_requestHdlr_acquire();
#ifdef _USE_LINUX_
    MerryProcess p;
    if (merry_create_process(&p) == mfalse)
    {
        merry_requestHdlr_release();
        c->registers[Ma] = 1;
        return RET_FAILURE;
    }
    if (p.pid == 0)
    {
        char entry[32];
        snprintf(entry, 32, "%llu", c->entry_addr);
        mstr_t argv[_len + 3];
        argv[0] = _MERRY_CMAIN_;
        argv[1] = entry;
        msize_t i = 2;
        for (; i < _len; i++)
        {
            argv[i] = *_args;
            _args++;
        }
        argv[i] = NULL;
        execv(/*Do something about this*/ _MERRY_CMAIN_, argv);
        exit(EXIT_FAILURE);
    }
#elif _USE_WIN_
    /// TODO: FIX ME
    merry_os_set_env(request->id);
    if (merry_create_process(&p) == mfalse)
    {
        merry_requestHdlr_release();
        os->cores[request->id]->registers[Ma] = 1;
        return RET_FAILURE;
    }
#endif
    merry_requestHdlr_release();
    os->cores[request->id]->registers[Ma] = 0;
    merry_os_notify_dbg(_NEW_OS_, 0);
    free(_args);
    return RET_SUCCESS;
}

_os_exec_(bp)
{
    merry_os_notify_dbg(_HIT_BP_, request->id);
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
        c->registers[Ma] = 1;
        if (!(func_name == NULL))
            free(func_name);
        return RET_FAILURE;
    }
    if ((function = merry_loader_getFuncSymbol(c->registers[Ma], func_name)) == RET_NULL)
    {
        merry_set_errno(MERRY_DYNERR);
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
        c->registers[Ma] = 1;
        if (!(func_name == NULL))
            free(func_name);
        return RET_FAILURE;
    }
    if ((function = merry_loader_getFuncSymbol(os->cores[request->id]->registers[Ma], func_name)) == RET_NULL)
    {
        merry_set_errno(MERRY_DYNERR);
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
        c->registers[Ma] = 1;
        return RET_FAILURE;
    }
    if (merry_loader_is_still_valid(c->registers[Ma]) == mfalse)
    {
        merry_set_errno(MERRY_DYNCLOSED);
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
    if (id == (mqword_t)-1)
    {
        merry_requestHdlr_panic(MERRY_INTERNAL_ERROR, 0);
        return RET_FAILURE;
    }
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
        return RET_FAILURE;
    }
    if (p.pid == 0)
    {
        char rfd[10];
        char wfd[10];
        char _id[10];
        snprintf(rfd, sizeof(rfd), "%d", channel->send_pipe->_read_fd);
        snprintf(wfd, sizeof(wfd), "%d", channel->receive_pipe->_write_fd);
        snprintf(_id, sizeof(_id), "%llu", id);
        mstr_t const argv[] = {_MERRY_SUBSYSMAIN_, rfd, wfd, _id, name, NULL};
        execv(/*Do something about this*/ _MERRY_SUBSYSMAIN_, argv);
        printf("EXECV ERROR\n");
        exit(EXIT_FAILURE);
    }
#elif _USE_WIN_
    /// TODO: Update this to work on windows too.
    if (merry_create_process(&p) == mfalse)
    {
        c->registers[Ma] = -1;
        return RET_FAILURE;
    }
#endif
    char t[2] = {_SUBSYS_ADD, id & 255};
    merry_config_channel(channel);
    // merry_(os->os_pipe, t, 2);
    merry_os_notify_subsys(t, 2);
    free(name);
    c->registers[Ma] = id;
    return RET_SUCCESS;
err:
    c->registers[Ma] = -1;
    return RET_FAILURE;
}

_os_exec_(close_channel)
{
    register MerryCore *c = os->cores[request->id];
    if (os->_subsystem_running == mfalse || os->_subsystem_failure == mtrue)
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
    if (merry_subsys_write(c->registers[Ma], c->registers[Mb], c->registers[M1], c->registers[M2], c->registers[M3], c->registers[M4]) == RET_FAILURE)
    {
        c->registers[Ma] = 1;
        return RET_FAILURE;
    }
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
    merry_subsys_add_task(c->registers[Mb], c->cond, &c->registers[Ma]);
    if (merry_subsys_write(c->registers[Ma], c->registers[Mb], c->registers[M1], c->registers[M2], c->registers[M3], c->registers[M4]) == RET_FAILURE)
    {
        c->registers[Ma] = 1;
        return RET_FAILURE;
    }
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

_os_exec_(get_core_count)
{
    // send the core count
    merry_os_notify_dbg(_REPLY_, os->core_count);
}

_os_exec_(get_active_core_count)
{
    merry_os_notify_dbg(_REPLY_, os->active_core_count);
}

_os_exec_(get_data_mem_page_count)
{
    merry_os_notify_dbg(_REPLY_, os->data_mem->number_of_pages);
}

_os_exec_(get_inst_mem_page_count)
{
    merry_os_notify_dbg(_REPLY_, os->inst_mem->number_of_pages);
}

_os_exec_(add_breakpoint)
{
    mqword_t _bp = OP_INTR;
    _bp <<= 56;
    _bp |= _REQ_BP;
    merry_memory_write(os->inst_mem, request->id, _bp); // if we fail, we fail
}

_os_exec_(inst_at)
{
    mqword_t inst = 0;
    merry_memory_read(os->inst_mem, request->id, &inst);
    merry_os_notify_dbg(_REPLY_, inst);
}

_os_exec_(data_at)
{
    mqword_t data = 0;
    merry_dmemory_read_qword(os->data_mem, request->id, &data);
    merry_os_notify_dbg(_REPLY_, data);
}

_os_exec_(sp_of)
{
    merry_os_notify_dbg(_REPLY_, os->cores[request->id & os->core_count]->sp);
}

_os_exec_(bp_of)
{
    merry_os_notify_dbg(_REPLY_, os->cores[request->id & os->core_count]->bp);
}

_os_exec_(pc_of)
{
    merry_os_notify_dbg(_REPLY_, os->cores[request->id & os->core_count]->pc);
}

_os_exec_(regr_of)
{
    merry_os_notify_dbg(_REPLY_, os->cores[request->id & os->core_count]->registers[request->id & 16]);
}

_os_exec_(continue_core)
{
    merry_cond_signal(os->cores[request->id & os->core_count]->cond);
}

_os_exec_(bt_of)
{
    // firstly, return how many stack frames there are
    // This way the debugger can anticipate how many bytes to expect
    size_t core_id = request->id;
    MerryCore *core = os->cores[core_id & os->core_count];
    merry_os_notify_dbg(_REPLY_, core->ras->sp + 1);
    for (msize_t i = 0; i <= core->ras->sp; i++)
    {
        merry_os_notify_dbg(_REPLY_, *(core->ras->array + i));
    }
}

_os_exec_(read_data_pg)
{
    msize_t pg_ind = request->id;
    msize_t pg_st_addr = pg_ind * _MERRY_MEMORY_ADDRESSES_PER_PAGE_;
    if (merry_reader_read_data_page(os->reader, pg_st_addr) != RET_SUCCESS)
    {
        // this is a proper error[The OS will handle it right here as internal error]
        mreport("VM failed to retrieve the requested data page at demand.");
        os->stop = mtrue;
        os->ret = RET_FAILURE;
        os->data_mem->error = MERRY_MEM_INVALID_ACCESS; // set to something as an indication
        return RET_FAILURE;
    }
    os->data_mem->pages[pg_ind]->address_space = os->reader->data[pg_ind]; // already initialized(we don't use madvise here because this page will be immediately used anyways)
    // in case of data memory, we will read three pages at once
    pg_ind--;
    if (merry_reader_read_data_page(os->reader, pg_ind * _MERRY_MEMORY_ADDRESSES_PER_PAGE_) != RET_SUCCESS)
    {
        // this is a proper error[The OS will handle it right here as internal error]
        mreport("VM failed to retrieve the requested data page at demand.");
        os->stop = mtrue;
        os->ret = RET_FAILURE;
        os->data_mem->error = MERRY_MEM_INVALID_ACCESS; // set to something as an indication
        return RET_FAILURE;
    }
    pg_ind += 2;
    if (os->reader->data_page_count == pg_ind)
        return RET_SUCCESS;
    if (merry_reader_read_data_page(os->reader, pg_ind * _MERRY_MEMORY_ADDRESSES_PER_PAGE_) != RET_SUCCESS)
    {
        // this is a proper error[The OS will handle it right here as internal error]
        mreport("VM failed to retrieve the requested data page at demand");
        os->stop = mtrue;
        os->ret = RET_FAILURE;
        os->data_mem->error = MERRY_MEM_INVALID_ACCESS; // set to something as an indication
        return RET_FAILURE;
    }
    os->data_mem->pages[pg_ind]->address_space = os->reader->data[pg_ind];
}

_os_exec_(read_inst_pg)
{
    msize_t pg_ind = request->id;
    MerrySection s = os->reader->sst.sections[pg_ind];
    if (merry_reader_read_inst_page(os->reader, os->reader->inst.instructions[pg_ind], pg_ind) != RET_SUCCESS)
    {
        mreport("VM failed to retrieve the requested instruction page at demand");
        os->stop = mtrue;
        os->ret = RET_FAILURE;
        os->inst_mem->error = MERRY_MEM_INVALID_ACCESS; // set to something as an indication
        return RET_FAILURE;
    }
    os->inst_mem->pages[pg_ind]->address_space = os->reader->inst.instructions[pg_ind];
}

void merry_os_notify_dbg(mqword_t sig, mqword_t arg)
{
    if (os.dbg_running != mtrue)
        return;
    merry_send_to_debugger(os.dbg, sig, arg);
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
    char finalf[128] = {'d', 'u', 'm', 'p', 0};
    snprintf(finalf + 4, 128, "%d", os._os_id);
    FILE *df = fopen(finalf, "w");
    if (df == NULL)
        return RET_FAILURE;
    // The details about the OS
    fprintf(df, "MANAGING THREAD[PROCESS]\n");
    fprintf(df, "\tCore count(At the time of termination): %lu\n", os.core_count);
    fprintf(df, "\tCause of Termination: CORE ID=%lu\n\n", os.err_core_id);
    merry_os_dump_mem_dets(df);
    merry_os_dump_core_dets(df);
    fclose(df);
    return RET_SUCCESS;
}

mqword_t merry_os_get_ret()
{
    return os.ret;
}

void merry_os_give_id(msize_t id)
{
    os._os_id = id;
}