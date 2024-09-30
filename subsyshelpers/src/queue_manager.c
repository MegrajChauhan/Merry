#include "queue_manager.h"

#ifdef _USE_LINUX_
mbool_t queue_manager_init(cond_t *_notifier, Queue *queue, int wfd, int rfd, msize_t id)
{
    if (init == mtrue)
        return mtrue;
    if ((manager.lock = lock_init()) == RET_NULL)
        return mfalse;
    init = mtrue;
    manager.queue = queue;
    manager.rfd = rfd;
    manager.wfd = wfd;
    manager._id = id;
    manager._notfier_cond = _notifier;
    manager._should_stop = mfalse;
    return mtrue;
}
#endif

void queue_manager_signal()
{
    atomic_store(&manager._should_stop, mtrue);
}

_THRET_T_ queue_manager(mptr_t arg)
{
    while (manager._should_stop != mtrue)
    {
        acquire_lock(manager.lock);
        mqword_t requests[5];
        printf("Waiting...\n");
        read(manager.rfd, requests, 40);
        printf("Got it!...\n");
        if (push_to_queue(manager.queue, requests[0], &requests[1]) == mfalse) // don't check return
        {
            printf("Failed to push...\n");
        }
        if (manager.queue->data_count == 1)
        {
            printf("Waking up the thread....\n");
            cond_signal(manager._notfier_cond);
        }
        if (requests[0] == 0)
        {
            release_lock(manager.lock);
            break;
        }
        release_lock(manager.lock);
    }
}

void queue_manager_destroy()
{
    // Leave the destruction of the queue and the notifier condition to the manager
    if (init == mfalse)
        return;
    lock_destroy(manager.lock);
    queue_destroy(manager.queue);
    cond_destroy(manager._notfier_cond);
}

mbool_t queue_send_reply(msize_t request, msize_t ret)
{
    mqword_t reply[2];
    reply[0] = request;
    reply[1] = ret;
    write(manager.wfd, reply, 16);
}

mbool_t queue_get_request(QueueItem *storein)
{
    mbool_t ret = pop_from_queue(manager.queue, storein);
    return ret;
}