#ifndef _MERRY_QUEUE_
#define _MERRY_QUEUE_

#include <stdlib.h>
#include <merry_types.h>
#include <merry_utils.h>
#include "merry_state.h"

// The queues store items as pointers

/**
 * MerryDynamicQueue is a dynamic queue whose length is dynamic.
 * It can contain as much data as required as long as there is memory to be fulfill the requests.
 * The drawback of this queue is that it is not that speed efficient.
 * One usecase for this queue may be when:
 * -- We need to pass a series of instructions to something. Here we make a queue of everything to be done at once and send everything at once to make things easier.
 * This queue is useful for cases when we need to use it only once.
 */
typedef struct MerryDynamicQueue MerryDynamicQueue;

/**
 * MerryStaticQueue is static in size but more faster on a long-term use.
 * This queue will shine if we need a queue that is used frequently.
 * The main drawback is the high memory consumption that it demands during initialization.
 */
typedef struct MerryStaticQueue MerryStaticQueue;

typedef struct MerryQueueNode MerryQueueNode; // We use Linked-list based queues

struct MerryQueueNode
{
    mptr_t data;
    MerryQueueNode *next_node;
    MerryQueueNode *prev;
};

struct MerryDynamicQueue
{
    // Linear linked list
    MerryQueueNode *head;
    MerryQueueNode *tail;
    msize_t data_count;
    MerryState qstate;
};

struct MerryStaticQueue
{
    // This is a circular linked list
    MerryQueueNode *head;
    MerryQueueNode *tail;
    msize_t data_count;
    msize_t capacity;
    MerryState qstate;
};

#define merry_is_dynamic_queue_empty(queue) ((queue)->data_count == 0)
#define merry_is_static_queue_empty(queue) ((queue)->data_count == 0)
#define merry_is_static_queue_full(queue) ((queue)->data_count >= (queue)->capacity)

MerryDynamicQueue *merry_dynamic_queue_init(MerryState *state);

mret_t merry_dynamic_queue_push(MerryDynamicQueue *queue, mptr_t data);

mret_t merry_dynamic_queue_pop(MerryDynamicQueue *queue, mptr_t *_store_in);

void merry_dynamic_queue_clear(MerryDynamicQueue *queue);

void merry_dynamic_queue_destroy(MerryDynamicQueue *queue);

// The given capacity is final
MerryStaticQueue *merry_static_queue_init(msize_t capacity, MerryState *state);

mret_t merry_static_queue_push(MerryStaticQueue *queue, mptr_t data);

mret_t merry_static_queue_pop(MerryStaticQueue *queue, mptr_t *_store_in);

void merry_static_queue_clear(MerryStaticQueue *queue);

void merry_static_queue_destroy(MerryStaticQueue *queue);

#endif
