#ifndef _MERRY_OS_QUEUE_
#define _MERRY_OS_QUEUE_

#include <merry_request.h>
#include <merry_queue.h>
#include <merry_thread.h>
#include "merry_console.h"

typedef struct MerryRequestNode MerryRequestNode;
typedef struct MerryRequestQueue MerryRequestQueue;

_MERRY_CREATE_QUEUE_NODE_NOPTR_(MerryOSRequest, MerryRequestNode);

_MERRY_CREATE_QUEUE_NOPTR_(MerryRequestQueue, MerryRequestNode);

MerryRequestQueue *merry_request_queue_init(msize_t number_of_requests);

static _MERRY_ALWAYS_INLINE_ inline mbool_t merry_is_queue_full(MerryRequestQueue *queue)
{
    return _MERRY_IS_QUEUE_FULL_NOPTR_(queue) ? mtrue : mfalse;
}

static _MERRY_ALWAYS_INLINE_ inline mbool_t merry_is_queue_emtpy(MerryRequestQueue *queue)
{
    return _MERRY_IS_QUEUE_EMPTY_NOPTR_(queue) ? mtrue : mfalse;
}

void merry_request_queue_destroy(MerryRequestQueue *queue);

mbool_t merry_push_request(MerryRequestQueue *queue, MerryCond *_req_cond, msize_t req_num, msize_t id);

mbool_t merry_push_dbg_requests(MerryRequestQueue *queue, msize_t req, mbyte_t op, mbyte_t arg_id, maddress_t addr);

mbool_t merry_pop_request(MerryRequestQueue *queue, MerryOSRequest *dest);

void merry_panic_push(MerryRequestQueue *queue, merrot_t error);

#endif