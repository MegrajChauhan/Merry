#ifndef _MERRY_TASK_QUEUE_
#define _MERRY_TASK_QUEUE_

#include "merry_task.h"
#include "merry_queue.h"
#include "merry_config.h"
#include <stdlib.h>

typedef struct MerryTaskNode MerryTaskNode;
typedef struct MerryTaskQueue MerryTaskQueue;

_MERRY_CREATE_QUEUE_NODE_NOPTR_(MerryTask, MerryTaskNode);
_MERRY_CREATE_QUEUE_NOPTR_(MerryTaskQueue, MerryTaskNode);

static inline mbool_t merry_is_task_queue_full(MerryTaskQueue *queue)
{
    return _MERRY_IS_QUEUE_FULL_NOPTR_(queue) ? mtrue : mfalse;
}

static inline mbool_t merry_is_task_queue_empty(MerryTaskQueue *queue)
{
    return _MERRY_IS_QUEUE_EMPTY_NOPTR_(queue) ? mtrue : mfalse;
}

MerryTaskQueue *merry_task_queue_init(msize_t queue_len);

void merry_task_queue_destroy(MerryTaskQueue *queue);

mbool_t merry_push_task(MerryTaskQueue *queue, MerryCond *cond, msize_t req, mqptr_t _store_in);

mbool_t merry_pop_task(MerryTaskQueue *queue, MerryTask *dest);

MerryTask *merry_query_tasks(MerryTaskQueue *queue, msize_t req);

#endif