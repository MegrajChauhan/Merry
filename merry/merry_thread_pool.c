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
    if ((pool->cond = merry_cond_init()) == RET_NULL)
    {
        free(pool->threads);
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

mptr_t merry_thPool_exec_func(mptr_t thread)
{
    MerryThreadPoolThread *th = (MerryThreadPoolThread *)thread;
    // now this thread will run
    while (mtrue)
    {
        if (th->stop == mtrue)
        {
            break;
        }
        // wait until we are assigned a job
        while (atomic_load(&th->state) != WORKING)
        {
        }

        // check if we need to stop
        if (th->stop == mtrue)
        {
            break;
        }

        // if we get here then it means that we were assigned a job
        switch (th->type)
        {
        case INP:
        {
            // this is an input operation
            th->exec_func.exec_IO(th->arg.IOarg_t.mem, th->arg.IOarg_t.mem); // now we expect the service function to set the correct registers for success indication
            break;
        }
        case OUT:
        {
            th->exec_func.exec_IO(th->arg.IOarg_t.mem, th->arg.IOarg_t.addr);
            break;
        }
        }
        // the job is done and the thread is free once again
        th->state = FREE;
        merry_cond_signal(th->pool_cond); // tell the pool that it can accept more threads
    }
    return RET_NULL;
}

mret_t merry_thPool_start_thread(MerryThreadPool *pool, msize_t id)
{
    if ((pool->threads[id].thread = merry_thread_init()) == RET_NULL)
        return RET_FAILURE;
    pool->threads[id]._is_init = mtrue;
    pool->threads[id].state = FREE;
    pool->threads[id].stop = mfalse;
    if (merry_create_detached_thread(pool->threads[id].thread, &merry_thPool_exec_func, &pool->threads[id]) == RET_FAILURE)
        return RET_FAILURE;
    pool->free_thread_count--;
    return RET_SUCCESS;
}

_MERRY_INTERNAL_ msize_t merry_thPool_get_free_thread(MerryThreadPool *pool)
{
    // if there are none, we simply
}

mret_t merry_assign_thread(MerryThPoolExec exec_func, MerryThPoolExecArg args)
{
    // assign a free thread a new task
}

void merry_destroy_thread_pool(MerryThreadPool *pool)
{
    if (pool == NULL)
        return;
    // we have to stop every thread as well
    merry_thPool_kill_threads(pool);
    for (msize_t i = 0; i < pool->pool_size; i++)
    {
        merry_thread_destroy(pool->threads[i].thread);
    }
    merry_cond_destroy(pool->cond);
    free(pool->threads);
    free(pool);
}