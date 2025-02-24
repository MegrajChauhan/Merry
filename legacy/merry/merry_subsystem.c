#include "merry_subsystem.h"
#include <signal.h>

_MERRY_INTERNAL_ msize_t merry_subsys_find_inactive_channel()
{
    for (msize_t i = 0; i < subsys.subsys_count; i++)
    {
        if (subsys.channels[i].comms_active == mfalse)
            return i;
    }
    return subsys.subsys_count + 1;
}

mret_t merry_init_subsys(msize_t _expected_subsys_count)
{
    inlog("COMPONENT INIT: SubSystem initializing.");
    subsys.channels = (MerrySubChannel *)malloc(sizeof(MerrySubChannel) * _expected_subsys_count);
    if (subsys.channels == NULL)
    {
        mreport("Failed to allocate SubSystem");
        goto _failed;
    }
    if ((subsys.queue = merry_task_queue_init(20)) == RET_NULL)
    {
        mreport("Failed to INIT TASK QUEUE for SubSystem");
        free(subsys.channels);
        goto _failed;
    }
    if ((subsys.lock = merry_mutex_init()) == RET_NULL)
    {
        mreport("Failed to GET MUTEX LOCK for SubSystem");
        free(subsys.channels);
        merry_task_queue_destroy(subsys.queue);
    }
    for (msize_t i = 0; i < _expected_subsys_count; i++)
    {
        subsys.channels[i].comms_active = mfalse;
        subsys.channels[i].receive_pipe = NULL;
        subsys.channels[i].send_pipe = NULL;
    }
    subsys.subsys_active = 0;
    subsys.subsys_count = _expected_subsys_count;
    subsys._stop = mfalse;
_done:
    inlog("COMPONENT INIT: SubSystem initialization successful");
    return RET_SUCCESS;
_failed:
    inlog("COMPONENT INIT: SubSystem initialization failed");
    return RET_FAILURE;
}

void merry_subsys_add_ospipe(MerryPipe *p)
{
    subsys.os_pipe = p;
}

msize_t merry_subsys_add_channel()
{
    inlog("COMPONENT ACTION: Adding a new channel to SubSystem.");

    merry_mutex_lock(subsys.lock);

    msize_t res = merry_subsys_find_inactive_channel();
    if (res != (subsys.subsys_count + 1))
    {
        if ((subsys.channels[res].receive_pipe == NULL) &&
            (merry_create_channel_given(&subsys.channels[res]) == RET_FAILURE))
        {
            mreport("Failed to create channel for inactive slot.");
            res = (mqword_t)-1;
        }
        else if (merry_reactivate_channel(&subsys.channels[res]) == RET_FAILURE)
        {
            mreport("Failed to reactivate the inactive channel.");
            res = (mqword_t)-1;
        }
    }
    else
    {
        MerrySubChannel *temp = (MerrySubChannel *)malloc(sizeof(MerrySubChannel) * (subsys.subsys_count + 10));
        if (temp == NULL)
        {
            mreport("Memory allocation for new channels failed.");
            res = (mqword_t)-1;
        }
        else
        {
            memcpy(temp, subsys.channels, subsys.subsys_count * sizeof(MerrySubChannel));
            free(subsys.channels);
            subsys.channels = temp;

            res = subsys.subsys_count;
            for (msize_t i = res; i < res + 10; i++)
            {
                subsys.channels[i].comms_active = mfalse;
                subsys.channels[i].receive_pipe = NULL;
                subsys.channels[i].send_pipe = NULL;
            }
            subsys.subsys_count += 10;

            if ((merry_create_channel_given(&subsys.channels[res])) == RET_FAILURE)
            {
                mreport("Failed to create new channel in expanded list.");
                res = (mqword_t)-1;
            }
            else
                merry_config_channel(&subsys.channels[res]);
        }
    }

    merry_mutex_unlock(subsys.lock);
    if (res == (mqword_t)-1)
        mreport("COMPONENT ACTION: Failed to add new channel.");
    else
        inlog("COMPONENT ACTION: Successfully added new channel.");
    return res;
}

MerrySubChannel *merry_subsys_get_channel(msize_t id)
{
    return &subsys.channels[id];
}

mret_t merry_subsys_add_task(msize_t request, MerryCond *cond, mqptr_t _store_in)
{
    return merry_push_task(subsys.queue, cond, request, _store_in) == mfalse ? RET_FAILURE : RET_SUCCESS;
}

mret_t merry_subsys_write(msize_t id, msize_t request, mqword_t arg1, mqword_t arg2, mqword_t arg3, mqword_t arg4)
{
    merry_mutex_lock(subsys.lock);
    if (surelyF(id >= subsys.subsys_count || subsys.channels[id].comms_active == mfalse))
    {
        merry_mutex_unlock(subsys.lock);
        return RET_FAILURE;
    }
    mqword_t buffer[5];
    buffer[0] = request;
    buffer[1] = arg1;
    buffer[2] = arg2;
    buffer[3] = arg3;
    buffer[4] = arg4;
    merry_channel_write(&subsys.channels[id], (mstr_t)buffer, 40);
    merry_mutex_unlock(subsys.lock);
    return RET_SUCCESS;
}

void merry_subsys_close_all()
{
    mqptr_t buffer[5];
    buffer[0] = _SUBSYS_CLOSE;
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0;
    buffer[4] = 0;
    for (msize_t id = 0; id < subsys.subsys_created; id++)
    {
        if (subsys.channels[id].comms_active == mtrue)
            merry_channel_write(&subsys.channels[id], (mstr_t)buffer, 40);
    }
}

void merry_destroy_subsys()
{
    massert(subsys.channels);
    for (size_t i = 0; i < subsys.subsys_created; i++)
        merry_close_channel(&subsys.channels[i]);
    free(subsys.channels);
    nil(subsys.channels);
    merry_mutex_destroy(subsys.lock);
    merry_task_queue_destroy(subsys.queue);
}

void merry_subsys_close_channel(msize_t id)
{
    if (surelyF(id >= subsys.subsys_count))
        return;
    mqptr_t buffer[5];
    buffer[0] = _SUBSYS_CLOSE;
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0;
    buffer[4] = 0;
    merry_channel_write(&subsys.channels[id], (mstr_t)buffer, 40);
}

_THRET_T_ merry_subsys_main(mptr_t arg)
{
#ifdef _USE_LINUX_
    inlog("SUBSYSTEM: Starting main loop.");
    merry_mutex_lock(subsys.lock);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        mreport("SUBSYSTEM INIT: Failed to create epoll instance.");
        merry_requestHdlr_panic(MERRY_SUBSYS_INIT_FAILURE, 0);
        merry_mutex_unlock(subsys.lock);
        goto err;
    }
    inlog("SUBSYSTEM INIT: epoll instance created successfully.");

    struct epoll_event _e = {.events = EPOLLIN, .data.fd = subsys.os_pipe->_read_fd};
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, subsys.os_pipe->_read_fd, &_e) == -1)
    {
        mreport("SUBSYSTEM INIT: Failed to add OS pipe to epoll.");
        merry_requestHdlr_panic(MERRY_SUBSYS_INIT_FAILURE, 0);
        merry_mutex_unlock(subsys.lock);
        goto err;
    }
    inlog("SUBSYSTEM INIT: OS pipe added to epoll successfully.");

    for (msize_t i = 0; i < subsys.subsys_count; i++)
    {
        if (!subsys.channels[i].comms_active || !subsys.channels[i].receive_pipe)
            continue;

        struct epoll_event ev = {.events = EPOLLIN, .data.fd = subsys.channels[i].receive_pipe->_read_fd};
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1)
        {
            mreport("SUBSYSTEM INIT: Failed to register channel with epoll.");
            merry_requestHdlr_panic(MERRY_SUBSYS_INIT_FAILURE, 0);
            merry_mutex_unlock(subsys.lock);
            goto err;
        }
        log("SUBSYSTEM INIT: Channel %zu registered with epoll.", i);
    }
    merry_mutex_unlock(subsys.lock);

    // Main event loop
    while (!subsys._stop)
    {
        struct epoll_event events[subsys.subsys_count + 1];
        int fd_count = epoll_wait(epoll_fd, events, subsys.subsys_count + 1, -1);
        if (fd_count == -1)
        {
            mreport("SUBSYSTEM ERROR: epoll_wait failed.");
            break;
        }
        log("SUBSYSTEM: epoll_wait returned with %d file descriptors ready.", fd_count);

        merry_mutex_lock(subsys.lock);
        for (int i = 0; i < fd_count; i++)
        {
            if (events[i].data.fd == subsys.os_pipe->_read_fd)
            {
                inlog("SUBSYSTEM: OS request received.");
                mbyte_t req;
                read(subsys.os_pipe->_read_fd, &req, 1);

                switch (req)
                {
                case _SUBSYS_SHUTDOWN:
                    inlog("SUBSYSTEM: Shutdown request received.");
                    merry_subsys_close_all();
                    merry_os_subsys_stopped();
                    subsys._stop = mtrue;
                    goto err;

                case _SUBSYS_ADD:
                    inlog("SUBSYSTEM: Add request received.");
                    read(subsys.os_pipe->_read_fd, &req, 1);
                    _e.data.fd = subsys.channels[req].receive_pipe->_read_fd;
                    if ((subsys.channels[req].comms_active == mtrue) &&
                        (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, subsys.channels[req].receive_pipe->_read_fd, &_e) == -1))
                    {
                        mreport("SUBSYSTEM ERROR: Failed to add new channel to epoll.");
                        merry_mutex_unlock(subsys.lock);
                        goto err;
                    }
                    log("SUBSYSTEM: Channel %d added successfully.", req);
                    char _s = 1;
                    write(subsys.channels[req].send_pipe->_write_fd, &_s, 1);
                    break;
                }
            }
            else
            {
                log("SUBSYSTEM: Channel data received on fd %d.", events[i].data.fd);
                mbyte_t buf[16];
                if (read(events[i].data.fd, buf, 16) < 16)
                {
                    mreport("SUBSYSTEM ERROR[WARNING]: Invalid number of bytes received.");
                    continue;
                }
                mqword_t request = *(mqptr_t)buf;
                mqword_t ret = *(mqptr_t)(buf + 8);

                switch (request)
                {
                case _SUBSYS_FAILED:
                    inlog("SUBSYSTEM: Fatal failure detected.");
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &_e);
                    merry_requestHdlr_panic(MERRY_SUBSYS_FAILED, 0);
                    merry_mutex_unlock(subsys.lock);
                    goto err;

                case _SUBSYS_CLOSED:
                    log("SUBSYSTEM: Closing channel with fd %d.", events[i].data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &_e);
                    if (ret >= subsys.subsys_count)
                    {
                        mreport("SUBSYSTEM ERROR: Invalid channel ID during close.");
                        merry_requestHdlr_panic(MERRY_SUBSYS_FAILED, 0);
                        merry_mutex_unlock(subsys.lock);
                        goto err;
                    }
                    merry_inactivate_channel(&subsys.channels[ret]);
                    break;

                default:
                    log("SUBSYSTEM: Task request %llu received.", request);
                    MerryTask *task;
                    if ((task = merry_query_tasks(subsys.queue, request)) != RET_NULL)
                    {
                        *task->_store_in = ret;
                        merry_cond_signal(task->cond);
                        log("SUBSYSTEM: Task %llu signaled with response %llu.", request, ret);
                    }
                }
            }
        }
        merry_mutex_unlock(subsys.lock);
    }
#endif // _USE_LINUX_

err:
    inlog("SUBSYSTEM: Cleaning up tasks.");
    MerryTask t;
    while ((merry_pop_task(subsys.queue, &t)) != mfalse)
    {
        merry_cond_signal(t.cond);
        inlog("SUBSYSTEM: Task signaled during cleanup.");
    }
    inlog("SUBSYSTEM: Exiting main loop.");
#ifdef _USE_LINUX_
    return RET_NULL;
#elif defined(_USE_WIN_)
    return RET_SUCCESS;
#endif
}