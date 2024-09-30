#ifndef _MTSYNC_
#define _MTSYNC_

// If the subsystem has it's own abstraction library then it may define the following constant
// and define the abstract contruct names the same as below

#ifndef SUBSYS_SYNC_PROVIDED
#define USE_MERRY
#include "merry_thread.h"

typedef MerryCond cond_t;
typedef MerryMutex lock_t;

#ifdef SUBSYS_EXTENDED
// with attributes and stuff
lock_t *lock_init(mptr_t arg, ...);
cond_t *cond_init(mptr_t arg, ...);
#else
lock_t *lock_init();
cond_t *cond_init();
#endif

// Add anything else for your usecase.
// The manager only needs the provided ones.

void acquire_lock(lock_t *_l);
void release_lock(lock_t *_l);
void cond_signal(cond_t* _c);

void lock_destroy(lock_t *_l);
void cond_destroy(cond_t *_c);

#endif
#endif