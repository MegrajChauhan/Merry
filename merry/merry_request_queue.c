#include "internals/merry_request_queue.h"

MerryRequestQueue *merry_request_queue_init(msize_t number_of_requests)
{
    MerryRequestQueue *queue;
    _MERRY_QUEUE_INIT_(MerryRequestQueue, queue)
    if (queue == NULL)
        return RET_NULL; // failed to initialize
    mbool_t ret = mtrue;
    _MERRY_QUEUE_CREATE_NODES_(queue, MerryRequestNode, number_of_requests, ret)
    if (ret == mfalse)
    {
        _MERRY_DESTROY_QUEUE_(queue)
        return RET_NULL;
    }
    return queue; // success
}

void merry_request_queue_destroy(MerryRequestQueue *queue){
    // queue is most likely not NULL
    _MERRY_DESTROY_QUEUE_(queue)
    // that is all
}

_MERRY_ALWAYS_INLINE mbool_t merry_is_queue_full(MerryRequestQueue *queue)
{
    return _MERRY_IS_QUEUE_FULL_(queue) ? mtrue : mfalse;
}

mret_t merry_push_request(MerryRequestQueue *queue, MerryCond *_req_cond, msize_t req_num, msize_t id)
{
    if (merry_is_queue_full(queue) == mtrue)
    {
        // the queue is full and so we gotta have to deny this request
        // we do not other way of handling this
        // this situation should not occur but it could still happen
        return RET_FAILURE;
    }
    // create a new request
    MerryOSRequest new_req = {req_num, _req_cond, id};
}