#include "../thread/merry_thread.h"

MerryMutex *merry_mutex_init()
{
    MerryMutex *mutex = (MerryMutex *)malloc(sizeof(MerryMutex));
    if (mutex == NULL)
        return RET_NULL; // failure to allocate
#if defined(_MERRY_THREADS_POSIX_)
    if (pthread_mutex_init(&mutex->mutex, NULL) != 0)
    {
        free(mutex);
        return RET_NULL;
    }
#endif
    return mutex; // return if success
}

MerryCond *merry_cond_init()
{
    MerryCond *cond = (MerryCond *)malloc(sizeof(MerryCond));
    if (cond == NULL)
        return RET_NULL; // failure to allocate
#if defined(_MERRY_THREADS_POSIX_)
    if (pthread_cond_init(&cond->cond, NULL) != 0)
    {
        free(cond);
        return RET_NULL;
    }
#endif
    return cond; // return if success
}

MerryThread *merry_thread_init()
{
    MerryThread *thread = (MerryThread *)malloc(sizeof(MerryThread));
    if (thread == NULL)
        return RET_NULL;
    // we are simply initializing a MerryThread
    return thread;
}

void merry_mutex_destroy(MerryMutex *mutex)
{
    if (surelyF(mutex == NULL))
        return;
#if defined(_MERRY_THREADS_POSIX_)
    pthread_mutex_destroy(&mutex->mutex);
#endif
    free(mutex); // free the mutex
}

void merry_cond_destroy(MerryCond *cond)
{
    if (surelyF(cond == NULL))
        return;
#if defined(_MERRY_THREADS_POSIX_)
    pthread_cond_destroy(&cond->cond);
#endif
    free(cond); // free the mutex
}

void merry_thread_destroy(MerryThread *thread)
{
    if (surelyF(thread == NULL))
        return;
    free(thread);
}

void merry_mutex_lock(MerryMutex *mutex)
{
    if (surelyF(mutex == NULL))
        return;
#if defined(_MERRY_THREADS_POSIX_)
    pthread_mutex_lock(&mutex->mutex);
#endif
}

void merry_mutex_unlock(MerryMutex *mutex)
{
    if (surelyF(mutex == NULL))
        return;
#if defined(_MERRY_THREADS_POSIX_)
    pthread_mutex_unlock(&mutex->mutex);
#endif
}

void merry_cond_wait(MerryCond *cond, MerryMutex *lock)
{
    if (surelyF(cond == NULL || lock == NULL))
        return;
#if defined(_MERRY_THREADS_POSIX_)
    pthread_cond_wait(&cond->cond, &lock->mutex);
#endif
}

void merry_cond_signal(MerryCond *cond)
{
    if (surelyF(cond == NULL))
        return;
#if defined(_MERRY_THREADS_POSIX_)
    pthread_cond_signal(&cond->cond);
#endif
}

mret_t merry_create_detached_thread(MerryThread *thread, ThreadExecFunc func, void *arg)
{
    if (surelyF(thread == NULL || func == NULL))
        return RET_FAILURE;
#if defined(_MERRY_THREADS_POSIX_)
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0)
        return RET_FAILURE; // we failed
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
    {
        pthread_attr_destroy(&attr);
        return RET_FAILURE;
    }
    if (pthread_create(&thread->thread, &attr, func, arg) != 0)
    {
        pthread_attr_destroy(&attr);
        return RET_FAILURE;
    }
    pthread_attr_destroy(&attr);
#endif
    return RET_SUCCESS;
}

mret_t merry_create_thread(MerryThread *thread, ThreadExecFunc func, void *arg)
{
    if (surelyF(thread == NULL || func == NULL))
        return RET_FAILURE;
#if defined(_MERRY_THREADS_POSIX_)
    if (pthread_create(&thread->thread, NULL, func, arg) != 0)
        return RET_FAILURE;
#endif
    return RET_SUCCESS;
}

mret_t merry_thread_join(MerryThread *thread, void *return_val)
{
    if (surelyF(thread == NULL))
        return RET_FAILURE;
#if defined(_MERRY_THREADS_POSIX_)
    pthread_join(thread->thread, &return_val);
#endif
    return RET_SUCCESS;
}