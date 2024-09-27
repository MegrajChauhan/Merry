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
    
}