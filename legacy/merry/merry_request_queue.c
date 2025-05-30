#include "merry_request_queue.h"

MerryRequestQueue *merry_request_queue_init(msize_t number_of_requests)
{
    inlog("Creating a REQUEST QUEUE");
    MerryRequestQueue *queue;
    _MERRY_QUEUE_INIT_(MerryRequestQueue, queue);
    if (queue == NULL)
    {
        mreport("Failed to create a REQUEST QUEUE");
        return RET_NULL; // failed to initialize
    }
    mbool_t ret = mtrue;
    _MERRY_QUEUE_CREATE_NODES_NOPTR_(queue, MerryRequestNode, number_of_requests, ret);
    if (ret == mfalse)
    {
        mreport("Failed to create a REQUEST QUEUE[NODES]");
        _MERRY_DESTROY_QUEUE_NOPTR_(queue);
        return RET_NULL;
    }
    return queue; // success
}

void merry_request_queue_destroy(MerryRequestQueue *queue)
{
    // queue is most likely not NULL
    massert(queue);
    _MERRY_DESTROY_QUEUE_NOPTR_(queue);
    // that is all
}

mbool_t merry_push_request(MerryRequestQueue *queue, MerryCond *_req_cond, msize_t req_num, msize_t id)
{
    mbool_t ret = mtrue;
    // create a new request
    MerryOSRequest new_req = {req_num, _req_cond, id, 0, 0, 0};
    _MERRY_QUEUE_PUSH_NOPTR_(queue, new_req, ret);
    return ret; // should be mfalse only when the queue is full
}

mbool_t merry_push_dbg_requests(MerryRequestQueue *queue, msize_t req, mbyte_t op, mbyte_t arg_id, maddress_t addr)
{
    mbool_t ret = mtrue;
    // create a new request
    MerryOSRequest new_req = {req, NULL, 0, op, arg_id, addr};
    _MERRY_QUEUE_PUSH_NOPTR_(queue, new_req, ret);
    return ret;
}

mbool_t merry_pop_request(MerryRequestQueue *queue, MerryOSRequest *dest)
{
    mbool_t ret = mtrue;
    _MERRY_QUEUE_POP_NOPTR_(queue, *dest, ret);
    return ret; // should be mfalse only when the queue is empty
}

void merry_panic_push(MerryRequestQueue *queue, merrot_t error)
{
    MerryOSRequest req = {error, NULL, 0};
    _MERRY_QUEUE_PANIC_PUSH_NOPTR_(queue, req);
    queue->data_count++;
}