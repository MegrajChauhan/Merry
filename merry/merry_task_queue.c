#include "merry_task_queue.h"

MerryTaskQueue *merry_task_queue_init(msize_t queue_len)
{
    MerryTaskQueue *queue;
    _MERRY_QUEUE_INIT_(MerryTaskQueue, queue);
    if (queue == NULL)
    {
        mreport("Failed to create TASK QUEUE");
        return RET_NULL;
    }
    mbool_t ret = mtrue;
    _MERRY_QUEUE_CREATE_NODES_NOPTR_(queue, MerryTaskNode, queue_len, ret);
    if (ret == mfalse)
    {
        mreport("Failed to create TASK QUEUE NODES");
        _MERRY_DESTROY_QUEUE_NOPTR_(queue);
        return RET_NULL;
    }
    return queue;
}

void merry_task_queue_destroy(MerryTaskQueue *queue)
{
    massert(queue);
    _MERRY_DESTROY_QUEUE_NOPTR_(queue);
}

mbool_t merry_push_task(MerryTaskQueue *queue, MerryCond *cond, msize_t req, mqptr_t _store_in)
{
    massert(queue);
    mbool_t ret = mtrue;
    MerryTask t;
    t.cond = cond;
    t.request = req;
    t._store_in = _store_in;
    _MERRY_QUEUE_PUSH_NOPTR_(queue, t, ret);
    return ret;
}

mbool_t merry_pop_task(MerryTaskQueue *queue, MerryTask *dest)
{
    mbool_t ret = mtrue;
    _MERRY_QUEUE_POP_NOPTR_(queue, *dest, ret);
    return ret;
}

MerryTask *merry_query_tasks(MerryTaskQueue *queue, msize_t req)
{
    MerryTaskNode *head = queue->head;
    MerryTaskNode *tmp = queue->head;
    MerryTaskNode *found = NULL;
    do
    {
        if (head->value.request == req)
        {
            found = head;
            break;
        }
        head = head->next;
    } while (head->next != tmp);
    if (found == NULL)
        return RET_NULL;
    found->value.request = 0;
    queue->data_count--;
    return &found->value;
}