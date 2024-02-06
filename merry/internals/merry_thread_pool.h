#ifndef _MERRY_THREAD_POOL_
#define _MERRY_THREAD_POOL_

// Represents a thread pool usable by the OS for some services

#include "../../utils/merry_types.h"
#include "../../sys/merry_thread.h"
#include <stdlib.h>
#include <stdatomic.h>

typedef struct MerryThreadPoolThread MerryThreadPoolThread;
typedef struct MerryThreadPool MerryThreadPool;
typedef enum MerryThreadState MerryThreadState;

enum MerryThreadState
{
    WORKING,
    FREE,
    NOT_INIT,
};

_MERRY_DEFINE_FUNC_PTR_(mptr_t, merry_pool_exec_t, ...)

struct MerryThreadPoolThread
{
    MerryThread *thread;
    mbool_t stop;           // tell the thread to stop
    MerryThreadState state; // the state of the thread
    merry_pool_exec_t exec_func;
    // Mutexes and condition variables for each thread in the pool could be added
};

struct MerryThreadPool
{
    MerryThreadPoolThread *threads; // the pool of the threads
    msize_t pool_size;              // the pool's size
    msize_t free_thread_count;      // How many threads are free?
};

MerryThreadPool *merry_init_thread_pool(msize_t pool_size);

void merry_destroy_thread_pool(MerryThreadPool *pool);

#endif