#ifndef _MERRY_TASK_
#define _MERRY_TASK_

#include "merry_thread.h"
#include "merry_queue.h"

typedef struct MerryTask MerryTask;

struct MerryTask
{
    MerryCond *cond;
    msize_t request;
    mqptr_t _store_in;
};

#endif