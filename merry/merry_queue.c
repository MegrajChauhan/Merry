#include "merry_queue.h"

MerryDynamicQueue *merry_dynamic_queue_init()
{
    MerryDynamicQueue *queue = (MerryDynamicQueue *)malloc(sizeof(MerryDynamicQueue));
    if (!queue)
        return RET_NULL;

    queue->head = queue->tail = NULL;
    queue->data_count = 0;
    return queue;
}

mret_t merry_dynamic_queue_push(MerryDynamicQueue *queue, mptr_t data)
{
    // Push the data and put it in the last
    merry_check_ptr(queue);
    merry_check_ptr(data);

    MerryQueueNode *node = (MerryQueueNode *)malloc(sizeof(MerryQueueNode));
    if (!node)
        return RET_FAILURE;

    node->data = data;

    if (merry_is_dynamic_queue_empty(queue))
    {
        node->next_node = NULL;
        node->prev = NULL;
        queue->head = queue->tail = node;
    }
    else
    {
        node->next_node = NULL;
        node->prev = queue->tail;
        queue->tail->next_node = node;
        queue->tail = node;
    }
    queue->data_count++;
    return RET_SUCCESS;
}

mret_t merry_dynamic_queue_pop(MerryDynamicQueue *queue, mptr_t *_store_in)
{
    merry_check_ptr(queue);
    merry_check_ptr(_store_in);

    if (merry_is_dynamic_queue_empty(queue))
    {
        *_store_in = NULL;
        return RET_FAILURE;
    }

    MerryQueueNode *head = queue->head;
    *_store_in = head->data;
    if (queue->data_count == 1)
    {
        queue->tail = NULL;
        queue->head = NULL;
    }
    else
    {
        head->next_node->prev = NULL;
        queue->head = head->next_node;
    }
    free(head);
    queue->data_count--;
    return RET_SUCCESS;
}

void merry_dynamic_queue_clear(MerryDynamicQueue *queue)
{
    /**
     * This function cleans the queue but the queue itself is not freed
     */
    merry_check_ptr(queue);
    merry_assert(!(queue->head && !queue->tail)); // check if the pointers have been corrupted
    merry_assert(!(!queue->head && queue->tail)); // check if the pointers have been corrupted

    MerryQueueNode *curr = queue->head;
    if (queue->head)
    {
        while (curr != NULL)
        {
            MerryQueueNode *tmp = curr->next_node;
            free(curr);
            curr = tmp;
        }
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->data_count = 0;
}

void merry_dynamic_queue_destroy(MerryDynamicQueue *queue)
{
    merry_dynamic_queue_clear(queue);
    free(queue);
}

MerryStaticQueue *merry_static_queue_init(msize_t capacity)
{
    merry_assert((capacity != 0));
    MerryStaticQueue *queue = (MerryStaticQueue *)malloc(sizeof(MerryStaticQueue));
    if (!queue)
        return RET_NULL;

    MerryQueueNode *first = (MerryQueueNode *)malloc(sizeof(MerryQueueNode));
    if (!first)
    {
        free(queue);
        return RET_NULL;
    }
    queue->head = first;
    for (msize_t i = 1; i < capacity; i++)
    {
        MerryQueueNode *node = (MerryQueueNode *)malloc(sizeof(MerryQueueNode));
        if (!node)
        {
            queue->data_count = i;
            merry_static_queue_destroy(queue);
            return RET_NULL;
        }
        node->data = NULL;
        node->next_node = NULL;
        node->prev = first;
        first->next_node = node;
        first = node;
    }
    first->next_node = queue->head;
    queue->tail = queue->head;
    queue->head->prev = first;
    queue->data_count = 0;
    queue->capacity = capacity;
    return queue;
}

mret_t merry_static_queue_push(MerryStaticQueue *queue, mptr_t data)
{
    // Push the data and put it in the last
    merry_check_ptr(queue);
    merry_check_ptr(data);

    if (merry_is_static_queue_full(queue))
        return RET_FAILURE;

    MerryQueueNode *n = queue->tail;
    n->data = data;
    queue->tail = n->next_node;
    queue->data_count++;
    return RET_SUCCESS;
}

mret_t merry_static_queue_pop(MerryStaticQueue *queue, mptr_t *_store_in)
{
    merry_check_ptr(queue);
    merry_check_ptr(_store_in);

    if (merry_is_static_queue_empty(queue))
        return RET_FAILURE;

    MerryQueueNode *node = queue->head;
    *_store_in = node->data;
    queue->data_count--;
    queue->head = node->next_node;
    return RET_SUCCESS;
}

void merry_static_queue_clear(MerryStaticQueue *queue)
{
    merry_check_ptr(queue);

    // we don't have to free the nodes in this case
    queue->head = queue->tail;
    queue->data_count = 0;
}

void merry_static_queue_destroy(MerryStaticQueue *queue)
{
    merry_check_ptr(queue);
    merry_assert(!(queue->head && !queue->tail)); // check if the pointers have been corrupted
    merry_assert(!(!queue->head && queue->tail)); // check if the pointers have been corrupted

    MerryQueueNode *curr = queue->head;
    if (queue->head)
    {
        while (curr != queue->tail)
        {
            MerryQueueNode *tmp = curr->next_node;
            free(curr);
            curr = tmp;
        }
    }
    free(queue);
}