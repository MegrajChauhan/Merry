#include <merry_queue_simple.h>

MerryQueueSimple *merry_create_simple_queue(msize_t cap, msize_t elen,
                                            MerryState *state) {
  if (surelyF(cap == 0 || elen == 0))
    return RET_NULL;
  MerryQueueSimple *queue =
      (MerryQueueSimple *)malloc(sizeof(MerryQueueSimple));
  if (!queue) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_NULL;
  }
  queue->buf = malloc(sizeof(mptr_t) * cap * elen);
  if (!queue->buf) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_NULL;
  }
  queue->buf_cap = cap;
  queue->elem_len = elen;
  queue->head = queue->rear = (mqword_t)(-1);
  return queue;
}

mptr_t merry_simple_queue_top(MerryQueueSimple *queue) {
  merry_check_ptr(queue);
  if (merry_simple_queue_empty(queue))
    return RET_NULL;
  return (mptr_t)(((char *)queue->buf + (queue->head * queue->elem_len)));
}

mret_t merry_simple_queue_enqueue(MerryQueueSimple *queue, mptr_t elem) {
  merry_check_ptr(queue);
  merry_check_ptr(elem);

  if (merry_simple_queue_full(queue))
    return RET_FAILURE;

  if (merry_simple_queue_empty(queue))
    queue->head = 0;
  queue->rear = (queue->rear + 1) % queue->buf_cap;
  memcpy((char *)queue->buf + queue->rear * queue->elem_len, elem,
         queue->elem_len);
  return RET_SUCCESS;
}

mret_t merry_simple_queue_dequeue(MerryQueueSimple *queue, mptr_t elem) {
  merry_check_ptr(queue);
  merry_check_ptr(elem);

  if (merry_simple_queue_empty(queue))
    return RET_FAILURE;

  memcpy(elem, (mptr_t)((char *)queue->buf + queue->head * queue->elem_len),
         queue->elem_len);

  if (queue->head == queue->rear)
    queue->head = queue->rear = (mqword_t)(-1);
  else
    queue->head = (queue->head + 1) % queue->buf_cap;

  return RET_SUCCESS;
}

void merry_destroy_simple_queue(MerryQueueSimple *queue) {
  merry_check_ptr(queue);
  merry_check_ptr(queue->buf);
  free(queue->buf);
  free(queue);
}
