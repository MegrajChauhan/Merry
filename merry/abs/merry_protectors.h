#ifndef _MERRY_PROTECTORS_
#define _MERRY_PROTECTORS_

#include <merry_config.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <merry_platform.h>

mret_t merry_mutex_init(mmutex_t *lock);

mret_t merry_cond_init(mcond_t *cond);

void merry_mutex_destroy(mmutex_t mutex);

void merry_cond_destroy(mcond_t cond);

void merry_mutex_lock(mmutex_t mutex);

void merry_mutex_unlock(mmutex_t mutex);

void merry_cond_wait(mcond_t cond, mmutex_t lock);

void merry_cond_signal(mcond_t cond);

void merry_cond_broadcast(mcond_t cond);

#endif