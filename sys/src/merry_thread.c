#include "../thread/merry_thread.h"

MerryMutex *merry_mutex_init()
{
    MerryMutex *mutex = (MerryMutex *)malloc(sizeof(MerryMutex));
    if (mutex == NULL)
        return RET_NULL; // failure to allocate
#if defined(_MERRY_THREADS_POSIX_)
    if (pthread_mutex_init(mutex->mutex, NULL) != 0)
    {
        free(mutex);
        return RET_NULL;
    }
#endif
    return mutex; // return if success
}

void merry_mutex_destroy(MerryMutex *mutex)
{
    if (surelyF(mutex == NULL))
        return;
#if defined(_MERRY_THREADS_POSIX_)
    pthread_mutex_destroy(mutex->mutex);
#endif
    free(mutex); // free the mutex
}