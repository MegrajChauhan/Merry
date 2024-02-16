#include "internals/merry_request_hdlr.h"

mret_t merry_requestHdlr_init(msize_t queue_len, MerryCond *cond)
{
    // _llog_(_REQHDLR_, "INIT", "Intializing request handler with queue length %lu", queue_len);
    req_hdlr.queue = merry_request_queue_init(queue_len);
    if (req_hdlr.queue == RET_NULL)
    {
        // _log_(_REQHDLR_, "FAILED", "Intializing request handler failed");
        return RET_FAILURE;
    }
    req_hdlr.lock = merry_mutex_init();
    if (req_hdlr.lock == RET_NULL)
    {
        // _log_(_REQHDLR_, "FAILED", "Intializing request handler failed");
        merry_request_queue_destroy(req_hdlr.queue);
        return RET_FAILURE;
    }
    req_hdlr.host_cond = cond;
    req_hdlr.handle_more = mtrue;
    // _log_(_REQHDLR_, "SUCCESS", "Request handler successfully initialized");
    return RET_SUCCESS;
}

mret_t merry_requestHdlr_push_request(msize_t req_id, msize_t id, MerryCond *req_cond)
{
    // _llog_(_REQHDLR_, "REQ_PUSH", "Core ID %lu pushing request %lu", id, req_id);
    mret_t success = RET_SUCCESS; // just to tell the core to stop furthur execution
    merry_mutex_lock(req_hdlr.lock);
    if (req_hdlr.handle_more == mfalse)
        goto here; // don't accept more
    if (merry_push_request(req_hdlr.queue, req_cond, req_id, id) == mfalse)
    {
        // _llog_(_REQHDLR_, "PANIC", "Request handler panic in core ID %lu", id);
        merry_panic_push(req_hdlr.queue, _PANIC_REQBUFFEROVERFLOW); // panic
        if (req_hdlr.queue->data_count == 1)
            merry_cond_signal(req_hdlr.host_cond); // wake up the OS
        success = RET_FAILURE;
    }
    else
    {
        // we succeeded
        // now we wait for the request to be fulfilled
        // _llog_(_REQHDLR_, "REQ_PUSH_SUCCESS", "Core ID %lu successfully pushed request", id);
        if (req_hdlr.queue->data_count == 1)
            merry_cond_signal(req_hdlr.host_cond); // wake up the OS
        // _llog_(_REQHDLR_, "WAITING", "Core ID %lu waiting for request to be fulfilled", id);
        merry_cond_wait(req_cond, req_hdlr.lock);  // the return value from the request should be in the requesting core's Registers
        // _llog_(_REQHDLR_, "DONE", "Core ID %lu request fulfilled. Waking up now", id);
    }
    goto here;
here:
    merry_mutex_unlock(req_hdlr.lock);
    return success;
}

void merry_requestHdlr_kill_requests()
{
    // _log_(_REQHDLR_, "PANIC", "Request Handler panicking; Killing requests");
    merry_mutex_lock(req_hdlr.lock);
    MerryOSRequest request;
    for (msize_t i = 0; i < req_hdlr.queue->data_count; i++)
    {
        merry_pop_request(req_hdlr.queue, &request);
        merry_cond_signal(request._wait_lock); // wake up the waiting core
    }
    merry_mutex_unlock(req_hdlr.lock);
}

void merry_requestHdlr_panic(merrot_t error)
{
    // _log_(_REQHDLR_, "PANIC", "Panic push requested");
    merry_mutex_lock(req_hdlr.lock);
    if (req_hdlr.handle_more == mfalse)
        goto here;                           // we are already panicking
    merry_panic_push(req_hdlr.queue, error); // panic push
    if (req_hdlr.queue->data_count == 1)
        merry_cond_signal(req_hdlr.host_cond); // wake up the OS if sleeping
    req_hdlr.handle_more = mfalse;             // don't accept any more requests
    goto here;
here:
    merry_mutex_unlock(req_hdlr.lock);
}

mbool_t merry_requestHdlr_pop_request(MerryOSRequest *request)
{
    // even the OS can't do anything while some core is pushing
    merry_mutex_lock(req_hdlr.lock);
    // in this case, failure would mean empty queue which ultimately tells the OS to go to sleep until a new request arrives
    mret_t ret = mfalse;
    // if (req_hdlr.handle_more == mfalse)
    //     goto here; // we shouldn't provide for any request now since we are in a state of panic[The OS should never call this function once a state of panic is entered]
    ret = merry_pop_request(req_hdlr.queue, request);
    goto here;
here:
    merry_mutex_unlock(req_hdlr.lock);
    return ret;
}

void merry_requestHdlr_destroy()
{
    // _log_(_REQHDLR_, "DESTROYING", "Destroying request handler");
    merry_mutex_destroy(req_hdlr.lock);
    merry_request_queue_destroy(req_hdlr.queue);
}