#ifndef _MERRY_REQUEST_HDLR_
#define _MERRY_REQUEST_HDLR_

#include <merry_request_queue.h>
#include <merry_errors.h>
#include <merry_thread.h>
#include "merry_console.h"
#include <stdlib.h>

typedef struct MerryRequestHdlr MerryRequestHdlr;

struct MerryRequestHdlr
{
    MerryRequestQueue *queue; // the request queue
    MerryMutex *lock;         // only one thread can pop and push
    MerryCond *host_cond;     // the OS's condition variable
    mbool_t handle_more;      // a flag to see if the handler should accept more request
};

// the request handler doesn't belong to even the OS just like Reader
static MerryRequestHdlr req_hdlr;

mret_t merry_requestHdlr_init(msize_t queue_len, MerryCond *cond);

mret_t merry_requestHdlr_push_request(msize_t req_id, msize_t id, MerryCond *req_cond);

void merry_requestHdlr_push_request_dbg(msize_t req, mbyte_t op, mbyte_t arg_id, maddress_t addr);

// exclusive for the OS
mbool_t merry_requestHdlr_pop_request(MerryOSRequest *request);

// exclusive for cores to inform the OS of errors quickly
void merry_requestHdlr_panic(merrot_t error, msize_t id);

// Make all cores that were waiting to continue with error
void merry_requestHdlr_kill_requests();

void merry_requestHdlr_destroy();

void merry_requestHdlr_empty();

void merry_requestHdlr_acquire();
void merry_requestHdlr_release();

#endif