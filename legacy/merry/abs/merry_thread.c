#include "merry_thread.h"

MerryMutex *merry_mutex_init()
{
    MerryMutex *mutex = (MerryMutex *)malloc(sizeof(MerryMutex));
    if (mutex == NULL)
    {
        // failure to allocate
        merry_set_errno(MERRY_VMERR);
        mreport("Failed to allocate a MUTEX");
        return RET_NULL;
    }
#if defined(_USE_LINUX_)
    if (pthread_mutex_init(&mutex->mutex, NULL) != 0)
    {
        free(mutex);
        mreport("Failed to allocate a MUTEX(SYS ERR)");
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
        mreport("Failed to allocate a COND_VAR");
        return RET_NULL; // failure to allocate
    }
#if defined(_USE_LINUX_)
    if (pthread_cond_init(&cond->cond, NULL) != 0)
    {
        free(cond);
        mreport("Failed to allocate a COND_VAR(SYS ERR)");
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
        mreport("Failed to allocate a THREAD");
        return RET_NULL;
    }
    // we are simply initializing a MerryThread
    return thread;
}

void merry_mutex_destroy(MerryMutex *mutex)
{
    massert(mutex != NULL);
#if defined(_USE_LINUX_)
    pthread_mutex_destroy(&mutex->mutex);
#elif defined(_USE_WIN_)
    DeleteCriticalSection(&mutex->mutex);
#endif
    free(mutex); // free the mutex
}

void merry_cond_destroy(MerryCond *cond)
{
    massert(cond != NULL);
#if defined(_USE_LINUX_)
    pthread_cond_destroy(&cond->cond);
#endif
    // for windows there is no need to destroy the condition variable
    free(cond); // free the mutex
}

void merry_thread_destroy(MerryThread *thread)
{
    massert(thread != NULL);
    free(thread);
}

void merry_mutex_lock(MerryMutex *mutex)
{
    massert(mutex != NULL);
#if defined(_USE_LINUX_)
    pthread_mutex_lock(&mutex->mutex);
#elif defined(_USE_WIN_)
    EnterCriticalSection(&mutex->mutex);
#endif
}

void merry_mutex_unlock(MerryMutex *mutex)
{
    massert(mutex != NULL);
#if defined(_USE_LINUX_)
    pthread_mutex_unlock(&mutex->mutex);
#elif defined(_USE_WIN_)
    LeaveCriticalSection(&mutex->mutex);
#endif
}

void merry_cond_wait(MerryCond *cond, MerryMutex *lock)
{
    massert(cond != NULL);
    massert(lock != NULL);
#if defined(_USE_LINUX_)
    pthread_cond_wait(&cond->cond, &lock->mutex);
#elif defined(_USE_WIN_)
    SleepConditionVariableCS(&cond->cond, &lock->mutex, INFINITE);
#endif
}

void merry_cond_signal(MerryCond *cond)
{
    massert(cond != NULL);
#if defined(_USE_LINUX_)
    pthread_cond_signal(&cond->cond);
#elif defined(_USE_WIN_)
    WakeConditionVariable(&cond->cond);
#endif
}

void merry_cond_broadcast(MerryCond *cond)
{
    massert(cond != NULL);
#if defined(_USE_LINUX_)
    pthread_cond_broadcast(&cond->cond);
#elif defined(_USE_WIN_)
    WakeAllConditionVariable(&cond->cond);
#endif
}

mret_t merry_create_detached_thread(MerryThread *thread, ThreadExecFunc func, void *arg)
{
    massert(thread != NULL);
    massert(func != NULL);
#if defined(_USE_LINUX_)
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0)
    {
        mreport("Failed to initialize DETACHED THREAD(SYS ERR)");
        return RET_FAILURE; // we failed
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
    {
        mreport("Failed to initialize DETACHED THREAD(SYS ERR)");
        pthread_attr_destroy(&attr);
        return RET_FAILURE;
    }
    if (pthread_create(&thread->thread, &attr, func, arg) != 0)
    {
        mreport("Failed to create DETACHED THREAD(SYS ERR)");
        pthread_attr_destroy(&attr);
        return RET_FAILURE;
    }
    pthread_attr_destroy(&attr);
#elif defined(_USE_WIN_)
    thread->thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL);
    if (thread->thread == NULL)
    {
        mreport("Failed to create DETACHED THREAD(SYS ERR)");
        return RET_FAILURE;
    }
#endif
    return RET_SUCCESS;
}

mret_t merry_create_thread(MerryThread *thread, ThreadExecFunc func, void *arg)
{
    massert(thread != NULL);
    massert(func != NULL);
#if defined(_USE_LINUX_)
    if (pthread_create(&thread->thread, NULL, func, arg) != 0)
    {
        mreport("Failed to create THREAD(SYS ERR)");
        return RET_FAILURE;
    }
#elif defined(_USE_WIN_)
    thread->thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL);
    if (thread->thread == NULL)
    {
        mreport("Failed to create THREAD(SYS ERR)");
        return RET_FAILURE;
    }
#endif
    return RET_SUCCESS;
}

mret_t merry_thread_join(MerryThread *thread, void *return_val)
{
    massert(thread != NULL);
#if defined(_USE_LINUX_)
    pthread_join(thread->thread, return_val);
#elif defined(_USE_WIN_)
    WaitForSingleObject(thread->thread, INFINITE);
#endif
    return RET_SUCCESS;
}