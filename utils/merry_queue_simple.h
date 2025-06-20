#ifndef _MERRY_QUEUE_SIMPLE_
#define _MERRY_QUEUE_SIMPLE_

#include "merry_state.h"
#include "merry_types.h"
#include "merry_utils.h"
#include <stdlib.h>

// Simple queue is going to be Circular queue
typedef struct MerryQueueSimple MerryQueueSimple;

struct MerryQueueSimple {
  msize_t head, rear;
  mptr_t *buf;
  msize_t buf_cap;
  msize_t elem_len;
};

#define merry_simple_queue_empty(queue) ((queue)->head == (mqword_t)(-1))
#define merry_simple_queue_full(queue)                                         \
  ((((queue)->rear + 1) % (queue)->buf_cap) == (queue)->head)
#define merry_simple_queue_clear(queue)                                        \
  ((queue)->head = (queue)->rear = (mqword_t)(-1))

MerryQueueSimple *merry_create_simple_queue(msize_t cap, msize_t elen,
                                            MerryState *state);

mptr_t merry_simple_queue_top(MerryQueueSimple *queue);

mret_t merry_simple_queue_enqueue(MerryQueueSimple *queue, mptr_t elem);

mret_t merry_simple_queue_dequeue(MerryQueueSimple *queue, mptr_t elem);

void merry_destroy_simple_queue(MerryQueueSimple *queue);

#endif
