#include "merry_subsystem.h"

_MERRY_INTERNAL_ msize_t merry_subsys_find_inactive_channel()
{
    for (msize_t i = 0; i < subsys.subsys_created; i++)
    {
        if (subsys.channels[i]->comms_active == mfalse)
            return i;
    }
    return subsys.subsys_count;
}

mret_t merry_init_subsys(msize_t _expected_subsys_count)
{
    subsys.channels = (MerrySubChannel **)malloc(sizeof(MerrySubChannel *) * _expected_subsys_count);
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
    subsys.subsys_active = 0;
    subsys.subsys_count = _expected_subsys_count;
    subsys.subsys_created = 0;
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
    if (res != subsys.subsys_count)
    {
        if (merry_reactivate_channel(subsys.channels[res]) == RET_FAILURE)
            res = (mqword_t)-1;
    }
    else
        // add another channel
        if (subsys.subsys_created == subsys.subsys_count)
        {
            MerrySubChannel **temp = (MerrySubChannel **)malloc(sizeof(MerrySubChannel *) * (subsys.subsys_count + 10));
            if (temp == NULL)
                res = (mqword_t)-1;
            else
            {
                memcpy(temp, subsys.channels, subsys.subsys_count * sizeof(MerrySubChannel *));
                free(subsys.channels);
                subsys.channels = temp;
                subsys.subsys_count += 10;
            }
        }
    if ((subsys.channels[subsys.subsys_created] = merry_create_channel()) == RET_NULL)
        res = (mqword_t)-1;
    else
    {
        res = subsys.subsys_created;
        subsys.subsys_created++;
    }
    merry_mutex_unlock(subsys.lock);
    return res;
}

MerrySubChannel *merry_subsys_get_channel(msize_t id)
{
    return subsys.channels[id];
}

mret_t merry_subsys_add_task(msize_t request, MerryCond *cond, mqptr_t _store_in)
{
    return merry_push_task(subsys.queue, cond, request, _store_in) == mfalse ? RET_FAILURE : RET_SUCCESS;
}

mret_t merry_subsys_write(msize_t id, msize_t request, mqword_t arg1, mqword_t arg2, mqword_t arg3, mqword_t arg4)
{
    merry_mutex_lock(subsys.lock);
    if (surelyF(id >= subsys.subsys_created || subsys.channels[id]->comms_active == mfalse))
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
    merry_channel_write(subsys.channels[id], (mstr_t)buffer, 40);
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
        if (subsys.channels[id]->comms_active == mtrue)
            merry_channel_write(subsys.channels[id], (mstr_t)buffer, 40);
    }
}

void merry_destroy_subsys()
{
    if (surelyF(subsys.channels == NULL))
        return;
    for (size_t i = 0; i < subsys.subsys_created; i++)
        merry_close_channel(subsys.channels[i]);
    free(subsys.channels);
    merry_mutex_destroy(subsys.lock);
    merry_task_queue_destroy(subsys.queue);
}

void merry_subsys_close_channel(msize_t id)
{
    if (surelyF(id >= subsys.subsys_created))
        return;
    mqptr_t buffer[5];
    buffer[0] = _SUBSYS_CLOSE;
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0;
    buffer[4] = 0;
    merry_channel_write(subsys.channels[id], (mstr_t)buffer, 40);
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
    for (msize_t i = 0; i < subsys.subsys_created; i++)
    {
        _e.data.fd = subsys.channels[i]->receive_pipe->_read_fd;
        if (subsys.channels[i]->comms_active == mtrue && epoll_ctl(epoll_fd, EPOLL_CTL_ADD, subsys.channels[i]->receive_pipe->_read_fd, &_e) == -1)
        {
            merry_requestHdlr_panic(MERRY_SUBSYS_INIT_FAILURE, 0);
            merry_mutex_unlock(subsys.lock);
            goto err;
        }
    }
    merry_mutex_unlock(subsys.lock);
    while (subsys._stop == mfalse)
    {
        int fd_count;
        merry_mutex_lock(subsys.lock);
        struct epoll_event events[subsys.subsys_created];
        merry_mutex_unlock(subsys.lock);
        fd_count = epoll_wait(epoll_fd, events, subsys.subsys_created, -1);
        merry_mutex_lock(subsys.lock);
        if (fd_count == -1)
        {
            merry_requestHdlr_panic(MERRY_SUBSYS_FAILED, 0);
            merry_mutex_unlock(subsys.lock);
            break;
        }
        for (msize_t i = 0; i < fd_count; i++)
        {
            if (events[i].data.fd == subsys.os_pipe->_read_fd)
            {
                // we have a request from the OS
                mbyte_t req;
                read(subsys.os_pipe->_read_fd, &req, 1);
                if (req == _SUBSYS_SHUTDOWN)
                {
                    subsys._stop = mtrue;
                    break;
                }
            }
            else
            {
                mbptr_t buf[16];
                read(events[i].data.fd, buf, 16);
                mqword_t request = *(mqptr_t)buf;
                mqword_t ret = *(mqptr_t)(buf + 8);
                switch (request)
                {
                case _SUBSYS_FAILED:
                {
                    // this failure is fatal
                    // we will do something like either shut down the VM
                    // or stop every other process and run without them
                    merry_requestHdlr_panic(MERRY_SUBSYS_FAILED, 0);
                    merry_mutex_unlock(subsys.lock);
                    goto err;
                }
                case _SUBSYS_CLOSED:
                {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &_e);
                    merry_inactivate_channel(subsys.channels[ret]);
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
    MerryTask t;
    while ((merry_pop_task(subsys.queue, &t)) != mfalse)
    {
        merry_cond_signal(t.cond);
    }
err:
#ifdef _USE_LINUX_
    return RET_NULL;
#elif defined(_USE_WIN_)
    return RET_SUCCESS;
#endif
}