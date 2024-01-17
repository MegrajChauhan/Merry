#ifndef _MERRY_OS_QUEUE_
#define _MERRY_OS_QUEUE_

#include "../lib/include/merry_queue.h"
#include "merry_request.h"

_MERRY_CREATE_QUEUE_NODE_(MerryOSRequest, MerryRequestNode)

_MERRY_CREATE_QUEUE_(MerryRequestQueue, MerryRequestNode)

typedef struct MerryRequestNode MerryRequestNode;
typedef struct MerryRequestQueue MerryRequestQueue;

MerryRequestQueue *merry_request_queue_init(msize_t number_of_requests);

void merry_request_queue_destroy(MerryRequestQueue *queue);

mret_t merry_push_request(MerryRequestQueue *queue, MerryCond *_req_cond, msize_t req_num, msize_t id);

#endif