#ifndef _QUEUE_MANAGER_
#define _QUEUE_MANAGER_

#include "queue.h"
#include "mtsync.h"
#include <stdatomic.h>
#include <stdlib.h>
#ifdef _USE_LINUX_
#include <unistd.h>
#endif

typedef struct QueueManager QueueManager;

struct QueueManager
{
    Queue *queue;
#ifdef _USE_LINUX_
    int wfd, rfd; // the handles
#endif
    msize_t _id;
    lock_t *lock;
    cond_t *_notfier_cond; // The subsystem may provide us with this and go to sleep in case there are no requests to handle
    mbool_t _should_stop;
};

_MERRY_INTERNAL_ QueueManager manager;
_MERRY_INTERNAL_ mbool_t init = mfalse;

/**
 * This manager will wait for any requests from the VM and notify the Subsystem if it is sleeping.
 * The subsystem may pop the requests and process them.
 * The subsystem may then send back the result as well 
 */

#ifdef _USE_LINUX_
mbool_t queue_manager_init(cond_t *_notifier, Queue *queue, int wfd, int rfd, msize_t id);
#endif
// use HANDLE for windows

void queue_manager_signal(); // signal to stop

_THRET_T_ queue_manager(mptr_t arg);

void queue_manager_destroy();

mbool_t queue_send_reply(msize_t request, msize_t ret);

mbool_t queue_get_request(QueueItem *storein);

#endif