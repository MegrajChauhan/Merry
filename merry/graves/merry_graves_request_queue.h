#ifndef _MERRY_GRAVES_REQUEST_QUEUE_
#define _MERRY_GRAVES_REQUEST_QUEUE_

#include "merry_core_base.h"
#include "merry_graves_constants.h"
#include "merry_protectors.h"
#include "merry_queue.h"
#include "merry_state.h"

typedef struct MerryGravesReqQueue MerryGravesReqQueue;
typedef struct MerryGravesRequest MerryGravesRequest;

struct MerryGravesRequest {
  mgreq_t type;
  MerryCoreBase *base;
  mqword_t args[4];
};

struct MerryGravesReqQueue {
  MerryState queue_state;
  MerryDynamicQueue *req_queue;
  mcond_t queue_cond;
  mmutex_t queue_lock;
  mcond_t *graves_cond;
  MerryGravesRequest req;
  mbool_t accept_requests;
};

_MERRY_LOCAL_ MerryGravesReqQueue g_queue;

mret_t merry_graves_req_queue_init(mcond_t *graves_cond, MerryState *state);

MerryState *merry_graves_req_queue_state();

mret_t merry_SEND_REQUEST(MerryGravesRequest *creq);

mret_t merry_graves_wants_work(MerryGravesRequest **req);

void merry_graves_req_queue_free();

#endif
