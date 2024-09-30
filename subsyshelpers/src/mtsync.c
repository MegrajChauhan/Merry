#include "mtsync.h"

// define the ones you want here yourself
lock_t *lock_init()
{
#ifdef USE_MERRY
    return merry_mutex_init();
#endif
    return RET_NULL;
}

cond_t *cond_init()
{
#ifdef USE_MERRY
    return merry_cond_init();
#endif
    return RET_NULL;
}

void acquire_lock(lock_t *_l)
{
#ifdef USE_MERRY
    return merry_mutex_lock(_l);
#endif
}

void release_lock(lock_t *_l)
{
#ifdef USE_MERRY
    return merry_mutex_unlock(_l);
#endif
}

void cond_signal(cond_t *_c)
{
#ifdef USE_MERRY
    return merry_cond_signal(_c);
#endif
}

void lock_destroy(lock_t *_l)
{
#ifdef USE_MERRY
    return merry_mutex_destroy(_l);
#endif
}

void cond_destroy(cond_t *_c)
{
#ifdef USE_MERRY
    return merry_cond_destroy(_c);
#endif
}