#include "merry_thread.h"

MerryMutex *merry_mutex_init()
{
    MerryMutex *mutex = (MerryMutex *)malloc(sizeof(MerryMutex));
    if (mutex == NULL)
    {
        // failure to allocate
        merry_set_errno(MERRY_VMERR);
        return RET_NULL;
    }
#if defined(_USE_LINUX_)
    if (pthread_mutex_init(&mutex->mutex, NULL) != 0)
    {
        free(mutex);
        merry_set_errno(MERRY_SYSERR);
        return RET_NULL;
    }
#elif defined(_USE_WIN_)
    // as mentioned in the documentation, this will always work with no errors
    InitializeCriticalSection(&mutex->mutex);
#endif
    return mutex; // return if success
}

MerryCond *merry_cond_init()
{
    MerryCond *cond = (MerryCond *)malloc(sizeof(MerryCond));
    if (cond == NULL)
    {
        merry_set_errno(MERRY_VMERR);
        return RET_NULL; // failure to allocate
    }
#if defined(_USE_LINUX_)
    if (pthread_cond_init(&cond->cond, NULL) != 0)
    {
        free(cond);
        merry_set_errno(MERRY_SYSERR);
        return RET_NULL;
    }
#elif defined(_USE_WIN_)
    InitializeConditionVariable(&cond->cond);
#endif
    return cond; // return if success
}

MerryThread *merry_thread_init()
{
    MerryThread *thread = (MerryThread *)malloc(sizeof(MerryThread));
    if (thread == NULL)
    {
        merry_set_errno(MERRY_VMERR);
        return RET_NULL;
    }
    // we are simply initializing a MerryThread
    return thread;
}

void merry_mutex_destroy(MerryMutex *mutex)
{
    if (surelyF(mutex == NULL))
        return;
#if defined(_USE_LINUX_)
    pthread_mutex_destroy(&mutex->mutex);
#elif defined(_USE_WIN_)
    DeleteCriticalSection(&mutex->mutex);
#endif
    free(mutex); // free the mutex
}

void merry_cond_destroy(MerryCond *cond)
{
    if (surelyF(cond == NULL))
        return;
#if defined(_USE_LINUX_)
    pthread_cond_destroy(&cond->cond);
#endif
    // for windows there is no need to destroy the condition variable
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
#if defined(_USE_LINUX_)
    pthread_mutex_lock(&mutex->mutex);
#elif defined(_USE_WIN_)
    EnterCriticalSection(&mutex->mutex);
#endif
}

void merry_mutex_unlock(MerryMutex *mutex)
{
    if (surelyF(mutex == NULL))
        return;
#if defined(_USE_LINUX_)
    pthread_mutex_unlock(&mutex->mutex);
#elif defined(_USE_WIN_)
    LeaveCriticalSection(&mutex->mutex);
#endif
}

void merry_cond_wait(MerryCond *cond, MerryMutex *lock)
{
    if (surelyF(cond == NULL || lock == NULL))
        return;
#if defined(_USE_LINUX_)
    pthread_cond_wait(&cond->cond, &lock->mutex);
#elif defined(_USE_WIN_)
    SleepConditionVariableCS(&cond->cond, &lock->mutex, INFINITE);
#endif
}

void merry_cond_signal(MerryCond *cond)
{
    if (surelyF(cond == NULL))
        return;
#if defined(_USE_LINUX_)
    pthread_cond_signal(&cond->cond);
#elif defined(_USE_WIN_)
    WakeConditionVariable(&cond->cond);
#endif
}

void merry_cond_broadcast(MerryCond *cond)
{
    if (surelyF(cond == NULL))
        return;
#if defined(_USE_LINUX_)
    pthread_cond_broadcast(&cond->cond);
#elif defined(_USE_WIN_)
    WakeAllConditionVariable(&cond->cond);
#endif
}

mret_t merry_create_detached_thread(MerryThread *thread, ThreadExecFunc func, void *arg)
{
    if (surelyF(thread == NULL || func == NULL))
        return RET_FAILURE;
#if defined(_USE_LINUX_)
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
#elif defined(_USE_WIN_)
    thread->thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL);
    if (thread->thread == NULL)
        return RET_FAILURE;
#endif
    return RET_SUCCESS;
}

mret_t merry_create_thread(MerryThread *thread, ThreadExecFunc func, void *arg)
{
    if (surelyF(thread == NULL || func == NULL))
        return RET_FAILURE;
#if defined(_USE_LINUX_)
    if (pthread_create(&thread->thread, NULL, func, arg) != 0)
        return RET_FAILURE;
#elif defined(_USE_WIN_)
    thread->thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL);
    if (thread->thread == NULL)
        return RET_FAILURE;
#endif
    return RET_SUCCESS;
}

mret_t merry_thread_join(MerryThread *thread, void *return_val)
{
    if (surelyF(thread == NULL))
        return RET_FAILURE;
#if defined(_USE_LINUX_)
    pthread_join(thread->thread, &return_val);
#elif defined(_USE_WIN_)
    WaitForSingleObject(thread->thread, INFINITE);
#endif
    return RET_SUCCESS;
}