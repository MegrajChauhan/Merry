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
typedef enum MerryThreadType MerryThreadType;
typedef union MerryThPoolExec MerryThPoolExec;
typedef union MerryThPoolExecArg MerryThPoolExecArg;

enum MerryThreadState
{
    WORKING = 5,
    FREE,
};

enum MerryThreadType
{
    INP,
    OUT,
};

_MERRY_DEFINE_FUNC_PTR_(mptr_t, merry_IO_exec_t, mptr_t, maddress_t)

union MerryThPoolExec
{
    merry_IO_exec_t exec_IO;
};

union MerryThPoolExecArg
{
    struct
    {
        mptr_t mem;
        maddress_t addr;
    } IOarg_t;
};

struct MerryThreadPoolThread
{
    MerryThread *thread;
    mbool_t stop;           // tell the thread to stop
    mbool_t _is_init;       // is the thread initialized?
    MerryThreadState state; // the state of the thread
    MerryThreadType type;   // the type of operation the thread is performing
    MerryThPoolExec exec_func;
    MerryThPoolExecArg arg;
    MerryCond *pool_cond; // the pool's cond
    // Mutexes and condition variables for each thread in the pool could be added
};

struct MerryThreadPool
{
    MerryThreadPoolThread *threads; // the pool of the threads
    msize_t pool_size;              // the pool's size
    msize_t free_thread_count;      // How many threads are free?
    MerryCond *cond;                // the pool's cond
};

MerryThreadPool *merry_init_thread_pool(msize_t pool_size);

mptr_t merry_thPool_exec_func(mptr_t thread);

mret_t merry_thPool_start_thread(MerryThreadPool *pool, msize_t id);

mret_t merry_assign_thread(MerryThPoolExec exec_func, MerryThPoolExecArg args);

void merry_destroy_thread_pool(MerryThreadPool *pool);

#endif