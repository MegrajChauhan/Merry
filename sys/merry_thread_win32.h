#ifndef _MERRY_THREAD_WIN32_
#define _MERRY_THREAD_WIN32_

#include <windows.h>

#define _THREAD_JOINABLE_ 0
#define _THREAD_DETACHED_ 1

/* Mutex */
struct MerryMutex
{
    CRITICAL_SECTION mutex;
};

/* Condition variable */
struct MerryCond
{
    CONDITION_VARIABLE cond;
};

/* Threads */
struct MerryThread
{
    HANDLE thread;
};

#endif