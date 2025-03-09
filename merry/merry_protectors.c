#include <merry_protectors.h>

mret_t merry_mutex_init(mmutex_t *lock)
{
    merry_check_ptr(lock);
#if defined(_USE_LINUX_)
    if (pthread_mutex_init(lock, NULL) != 0)
        return RET_FAILURE;

#elif defined(_USE_WIN_)
    // as mentioned in the documentation, this will always work with no errors
    InitializeCriticalSection(lock);
#endif
    return RET_SUCCESS; // return if success
}

mret_t merry_cond_init(mcond_t *cond)
{
    merry_check_ptr(cond);
#if defined(_USE_LINUX_)
    if (pthread_cond_init(cond, NULL) != 0)
        return RET_FAILURE;
#elif defined(_USE_WIN_)
    InitializeConditionVariable(cond);
#endif
    return cond; // return if success
}

void merry_mutex_destroy(mmutex_t mutex)
{
#if defined(_USE_LINUX_)
    pthread_mutex_destroy(&mutex);
#elif defined(_USE_WIN_)
    DeleteCriticalSection(&mutex);
#endif
}

void merry_cond_destroy(mcond_t cond)
{
#if defined(_USE_LINUX_)
    pthread_cond_destroy(&cond);
#endif
}

void merry_mutex_lock(mmutex_t mutex)
{
#if defined(_USE_LINUX_)
    pthread_mutex_lock(&mutex);
#elif defined(_USE_WIN_)
    EnterCriticalSection(&mutex);
#endif
}

void merry_mutex_unlock(mmutex_t mutex)
{
#if defined(_USE_LINUX_)
    pthread_mutex_unlock(&mutex);
#elif defined(_USE_WIN_)
    LeaveCriticalSection(&mutex);
#endif
}

void merry_cond_wait(mcond_t cond, mmutex_t lock)
{
#if defined(_USE_LINUX_)
    pthread_cond_wait(&cond, &lock);
#elif defined(_USE_WIN_)
    SleepConditionVariableCS(&cond, &lock, INFINITE);
#endif
}

void merry_cond_signal(mcond_t cond)
{
#if defined(_USE_LINUX_)
    pthread_cond_signal(&cond);
#elif defined(_USE_WIN_)
    WakeConditionVariable(&cond);
#endif
}

void merry_cond_broadcast(mcond_t cond)
{

#if defined(_USE_LINUX_)
    pthread_cond_broadcast(&cond);
#elif defined(_USE_WIN_)
    WakeAllConditionVariable(&cond);
#endif
}
