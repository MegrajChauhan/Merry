#ifndef _QUEUE_
#define _QUEUE_

#include "merry_types.h"
#include "merry_queue.h"

typedef struct QueueItem QueueItem;

struct QueueItem
{
    msize_t request;
    msize_t arg[4];
};

typedef struct QueueNode QueueNode;
typedef struct Queue Queue;

_MERRY_CREATE_QUEUE_NODE_NOPTR_(QueueItem, QueueNode);
_MERRY_CREATE_QUEUE_(Queue, QueueNode);

static _MERRY_ALWAYS_INLINE_ inline mbool_t queue_full(Queue *queue)
{
    return _MERRY_IS_QUEUE_FULL_NOPTR_(queue) ? mtrue : mfalse;
}

static _MERRY_ALWAYS_INLINE_ inline mbool_t queue_empty(Queue *queue)
{
    return _MERRY_IS_QUEUE_EMPTY_NOPTR_(queue) ? mtrue : mfalse;
}

Queue *queue_init(msize_t item_count);

void queue_destroy(Queue *queue);

mbool_t push_to_queue(Queue *queue, msize_t req_num, msize_t *args);

mbool_t pop_from_queue(Queue *queue, QueueItem *dest);

#endif