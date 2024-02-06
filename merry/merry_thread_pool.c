#include "internals/merry_thread_pool.h"

MerryThreadPool *merry_init_thread_pool(msize_t pool_size)
{
    MerryThreadPool *pool = (MerryThreadPool *)malloc(sizeof(MerryThreadPool));
    if (pool == NULL)
        return RET_NULL;
    pool->threads = (MerryThreadPoolThread *)malloc(sizeof(MerryThreadPoolThread) * pool_size);
    if (pool->threads == NULL)
    {
        free(pool);
        return RET_NULL;
    }
    pool->free_thread_count = pool_size; // all the threads are free
    pool->pool_size = pool_size;
    return pool;
}

_MERRY_INTERNAL_ void merry_thPool_kill_threads(MerryThreadPool *pool)
{
    // we kill all the threads
    if (pool->free_thread_count == pool->pool_size)
        return;
    if (pool->pool_size == 1)
        atomic_exchange(&pool->threads[0].stop, mtrue);
    else
    {
        for (msize_t i = 0; i < pool->pool_size; i++)
        {
            atomic_exchange(&pool->threads[i].stop, mtrue);
        }
    }
}

void merry_destroy_thread_pool(MerryThreadPool *pool)
{
    if (pool == NULL)
        return;
    // we have to stop every thread as well
    merry_thPool_kill_threads(pool);
    free(pool->threads);
    free(pool);
}