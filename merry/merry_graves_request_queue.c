#include "merry_graves_request_queue.h"

mret_t merry_graves_req_queue_init(mcond_t *graves_cond, MerryState *state) {
  if (merry_cond_init(&g_queue.queue_cond) == RET_FAILURE) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OBTAIN_COND_);
    return RET_FAILURE;
  }
  if (merry_mutex_init(&g_queue.queue_lock) == RET_FAILURE) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OBTAIN_LOCK_);
    merry_cond_destroy(&g_queue.queue_cond);
    return RET_FAILURE;
  }

  if ((g_queue.req_queue = merry_dynamic_queue_init(state)) == RET_NULL) {
    merry_cond_destroy(&g_queue.queue_cond);
    merry_mutex_destroy(&g_queue.queue_lock);
    return RET_FAILURE;
  }

  g_queue.queue_state.child_state = &g_queue.req_queue->qstate;
  g_queue.graves_cond = graves_cond;
  merry_assign_state(g_queue.queue_state, _MERRY_ORIGIN_NONE_, 0);
  g_queue.accept_requests = mtrue;
  return RET_SUCCESS;
}

MerryState *merry_graves_req_queue_state() { return &g_queue.queue_state; }

mret_t merry_SEND_REQUEST(MerryGravesRequest *creq) {
  if (g_queue.accept_requests == mfalse)
    return RET_FAILURE;
  merry_mutex_lock(&g_queue.queue_lock);

  if (merry_dynamic_queue_push(g_queue.req_queue, creq) == RET_FAILURE) {
    merry_assign_state(g_queue.queue_state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_GRAVES_REQUEST_QUEUE_FAILURE_);
    g_queue.queue_state.child_state = &g_queue.req_queue->qstate;
    merry_dynamic_queue_clear(g_queue.req_queue);
    g_queue.accept_requests = mfalse;
    merry_cond_signal(g_queue.graves_cond);
    return RET_FAILURE;
  }

  merry_cond_signal(g_queue.graves_cond);
  merry_cond_wait(&creq->base->cond, &g_queue.queue_lock);
  merry_mutex_unlock(&g_queue.queue_lock);
  return RET_SUCCESS;
}

mret_t merry_graves_wants_work(MerryGravesRequest **req) {
  merry_mutex_lock(&g_queue.queue_lock);
  mret_t res = merry_dynamic_queue_pop(g_queue.req_queue, (mptr_t *)req);
  merry_mutex_unlock(&g_queue.queue_lock);
  return res;
}

void merry_graves_req_queue_free() {
  merry_dynamic_queue_destroy(g_queue.req_queue);
  merry_cond_destroy(&g_queue.queue_cond);
  merry_mutex_destroy(&g_queue.queue_lock);
}
