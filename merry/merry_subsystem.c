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
    subsys.channels = (MerrySubChannel *)malloc(sizeof(MerrySubChannel) * _expected_subsys_count);
    if (subsys.channels == NULL)
        return RET_FAILURE;
    if ((subsys.queue = merry_task_queue_init(20)) == RET_NULL)
    {
        free(subsys.channels);
        return RET_FAILURE;
    }
    if ((subsys.lock = merry_mutex_init()) == RET_NULL)
    {
        free(subsys.channels);
        merry_task_queue_destroy(subsys.queue);
        return RET_FAILURE;
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
    return RET_SUCCESS;
}

void merry_subsys_add_ospipe(MerryPipe *p)
{
    subsys.os_pipe = p;
}

msize_t merry_subsys_add_channel()
{
    merry_mutex_lock(subsys.lock);
    msize_t res = merry_subsys_find_inactive_channel();
    if (res != (subsys.subsys_count + 1))
    {
        if ((subsys.channels[res].receive_pipe == NULL) && (merry_create_channel_given(&subsys.channels[res])) == RET_FAILURE)
            res = (mqword_t)-1;
        else if (merry_reactivate_channel(&subsys.channels[res]) == RET_FAILURE)
            res = (mqword_t)-1;
    }
    else
    {
        // add another channel
        MerrySubChannel *temp = (MerrySubChannel *)malloc(sizeof(MerrySubChannel) * (subsys.subsys_count + 10));
        if (temp == NULL)
            res = (mqword_t)-1;
        else
        {
            memcpy(temp, subsys.channels, subsys.subsys_count * sizeof(MerrySubChannel));
            free(subsys.channels);
            subsys.channels = temp;
            res = subsys.subsys_count;
            for (msize_t i = res; i < 10; i++)
            {
                subsys.channels[i].comms_active = mfalse;
                subsys.channels[i].receive_pipe = NULL;
                subsys.channels[i].send_pipe = NULL;
            }
            subsys.subsys_count += 10;
            if ((merry_create_channel_given(&subsys.channels[res])) == RET_FAILURE)
                res = (mqword_t)-1;
            else
                merry_config_channel(&subsys.channels[res]);
        }
    }
    merry_mutex_unlock(subsys.lock);
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
    if (surelyF(subsys.channels == NULL))
        return;
    for (size_t i = 0; i < subsys.subsys_created; i++)
        merry_close_channel(&subsys.channels[i]);
    free(subsys.channels);
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
    merry_mutex_lock(subsys.lock);
    int epoll_fd = epoll_create1(0);
    struct epoll_event _e;
    if (epoll_fd == -1)
    {
        merry_requestHdlr_panic(MERRY_SUBSYS_INIT_FAILURE, 0);
        merry_mutex_unlock(subsys.lock);
        goto err;
    }
    _e.events = EPOLLIN;
    _e.data.fd = subsys.os_pipe->_read_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, subsys.os_pipe->_read_fd, &_e) == -1)
    {
        merry_requestHdlr_panic(MERRY_SUBSYS_INIT_FAILURE, 0);
        merry_mutex_unlock(subsys.lock);
        goto err;
    }
    for (msize_t i = 0; i < subsys.subsys_count; i++)
    {
        struct epoll_event ev;
        ev.events = EPOLLIN;
        if (subsys.channels[i].receive_pipe == NULL)
            continue;
        ev.data.fd = subsys.channels[i].receive_pipe->_read_fd;
        if ((subsys.channels[i].comms_active == mtrue) && (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, subsys.channels[i].receive_pipe->_read_fd, &ev) == -1))
        {
            merry_requestHdlr_panic(MERRY_SUBSYS_INIT_FAILURE, 0);
            merry_mutex_unlock(subsys.lock);
            goto err;
        }
    }
    merry_mutex_unlock(subsys.lock);
    while (subsys._stop == mfalse)
    {
        int fd_count = 0;
        struct epoll_event events[subsys.subsys_count + 1];
        fd_count = epoll_wait(epoll_fd, events, subsys.subsys_count + 1, -1);
        if (fd_count == -1)
        {
            merry_requestHdlr_panic(MERRY_SUBSYS_INIT_FAILURE, 0);
            break;
        }
        merry_mutex_lock(subsys.lock);
        for (msize_t i = 0; i < fd_count; i++)
        {
            if (events[i].data.fd == subsys.os_pipe->_read_fd)
            {
                // we have a request from the OS
                mbyte_t req;
                read(subsys.os_pipe->_read_fd, &req, 1);
                switch (req)
                {
                case _SUBSYS_SHUTDOWN:
                {
                    merry_subsys_close_all();
                    merry_os_subsys_stopped();
                    subsys._stop = mtrue;
                    goto err;
                }
                case _SUBSYS_ADD:
                {
                    read(subsys.os_pipe->_read_fd, &req, 1);
                    _e.data.fd = subsys.channels[req].receive_pipe->_read_fd;
                    if ((subsys.channels[req].comms_active == mtrue) && (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, subsys.channels[req].receive_pipe->_read_fd, &_e) == -1))
                    {
                        merry_requestHdlr_panic(MERRY_SUBSYS_INIT_FAILURE, 0);
                        merry_mutex_unlock(subsys.lock);
                        goto err;
                    }
                    char _s = 1;
                    write(subsys.channels[req].send_pipe->_write_fd, &_s, 1);
                    break;
                }
                }
            }
            else
            {
                mbyte_t buf[16];
                if (read(events[i].data.fd, buf, 16) < 16)
                {
                    fprintf(stderr, "SUBSYSTEM ERROR[WARNING]: Invalid number of bytes received.\n");
                    continue;
                }
                mqword_t request = *(mqptr_t)buf;
                mqword_t ret = *(mqptr_t)(buf + 8);
                switch (request)
                {
                case _SUBSYS_FAILED:
                {
                    // this failure is fatal
                    // we will do something like either shut down the VM
                    // or stop every other process and run without them
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &_e);
                    merry_requestHdlr_panic(MERRY_SUBSYS_FAILED, 0);
                    merry_mutex_unlock(subsys.lock);
                    goto err;
                }
                case _SUBSYS_CLOSED:
                {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &_e);
                    if (ret >= subsys.subsys_count)
                    {
                        merry_requestHdlr_panic(MERRY_SUBSYS_FAILED, 0);
                        merry_mutex_unlock(subsys.lock);
                        goto err;
                    }
                    merry_inactivate_channel(&subsys.channels[ret]);
                    break;
                }
                default:
                    MerryTask *task;
                    if ((task = merry_query_tasks(subsys.queue, request)) != RET_NULL)
                    {
                        *task->_store_in = ret;
                        merry_cond_signal(task->cond);
                    }
                }
            }
        }
        merry_mutex_unlock(subsys.lock);
    }
#endif
err:
    MerryTask t;
    while ((merry_pop_task(subsys.queue, &t)) != mfalse)
    {
        merry_cond_signal(t.cond);
    }
#ifdef _USE_LINUX_
    return RET_NULL;
#elif defined(_USE_WIN_)
    return RET_SUCCESS;
#endif
}