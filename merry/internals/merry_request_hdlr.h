#ifndef _MERRY_REQUEST_HDLR_
#define _MERRY_REQUEST_HDLR_

#include "../../utils/merry_types.h"
// #include "../lib/include/merry_memory_allocator.h" <LEGACY>
#include "../../sys/merry_thread.h"
#include "merry_request_queue.h"
#include <stdlib.h>

typedef struct MerryRequestHdlr MerryRequestHdlr;

struct MerryRequestHdlr
{
    MerryRequestQueue *queue; // the request queue
    MerryMutex *lock;         // only one thread can pop and push
    MerryCond *host_cond;     // the OS's condition variable
};

// the request handler doesn't belong to even the OS just like Reader
static MerryRequestHdlr req_hdlr;

mret_t merry_requestHdlr_init(msize_t queue_len, MerryCond *cond);

mret_t merry_requestHdlr_push_request(msize_t req_id, msize_t id, MerryCond *req_cond);

// exclusive for the OS
mbool_t merry_requestHdlr_pop_request(MerryOSRequest *request);

void merry_requestHdlr_destroy();

#endif