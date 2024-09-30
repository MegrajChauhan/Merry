#include "queue.h"

Queue *queue_init(msize_t item_count)
{
    Queue *queue;
    _MERRY_QUEUE_INIT_(Queue, queue);
    if (queue == NULL)
        return RET_NULL;
    mbool_t ret = mtrue;
    _MERRY_QUEUE_CREATE_NODES_NOPTR_(queue, QueueNode, item_count, ret);
    if (ret == mfalse)
    {
        _MERRY_DESTROY_QUEUE_NOPTR_(queue);
        return RET_NULL;
    }
    return queue;
}

void queue_destroy(Queue *queue)
{
    _MERRY_DESTROY_QUEUE_NOPTR_(queue);
}

mbool_t push_to_queue(Queue *queue, msize_t req_num, msize_t *args)
{
    mbool_t ret = mtrue;
    QueueItem t;
    t.request = req_num;
    t.arg[0] = args[0];
    t.arg[1] = args[1];
    t.arg[2] = args[2];
    t.arg[3] = args[3];
    _MERRY_QUEUE_PUSH_NOPTR_(queue, t, ret);
    return ret;
}

mbool_t pop_from_queue(Queue *queue, QueueItem *dest)
{
    mbool_t ret = mtrue;
    _MERRY_QUEUE_POP_NOPTR_(queue, *dest, ret);
    return ret;
}