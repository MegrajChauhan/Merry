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
  mptr_t buf;
  msize_t elem_len;
  msize_t buf_cap;
};

#define merry_simple_queue_empty(queue)
#define merry_simple_queue_full(queue)

MerryQueueSimple *merry_create_simple_queue(msize_t elem_len, msize_t cap,
                                            MerryState *state);

mptr_t merry_simple_queue_at(MerryQueueSimple *queue, msize_t ind,
                             MerryState *state);

mret_t merry_simple_queue_enqueue(MerryQueueSimple *queue, mptr_t elem,
                                  MerryState *state);

mret_t merry_simple_queue_dequeue(MerryQueueSimple *queue, mptr_t elem,
                                  MerryState *state);

void merry_destroy_simple_queue(MerryQueueSimple *queue);

#endif
