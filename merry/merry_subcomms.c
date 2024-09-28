#include "merry_subcomms.h"

MerrySubChannel *merry_create_channel()
{
    MerryPipe *rpipe, *wpipe;
    if ((rpipe = merry_open_pipe()) == RET_NULL)
        return RET_NULL;
    if ((wpipe = merry_open_pipe()) == RET_NULL)
    {
        merry_destroy_pipe(rpipe);
        return RET_NULL;
    }
    MerrySubChannel *channel = (MerrySubChannel *)malloc(sizeof(MerrySubChannel));
    if (channel == NULL)
    {
        merry_destroy_pipe(wpipe);
        merry_destroy_pipe(rpipe);
        return RET_NULL;
    }
    // we can't configure the pipes just yet
    channel->send_pipe = wpipe;
    channel->receive_pipe = rpipe;
    return channel;
}

void merry_inactivate_channel(MerrySubChannel *channel)
{
    if (surelyF(channel == RET_NULL))
        return;
    channel->comms_active = mfalse;
    // close all ends
    // we will have to re-open if the channel needs to be reused
    merry_pipe_close_both_ends(channel->send_pipe);
    merry_pipe_close_both_ends(channel->receive_pipe);
}

void merry_close_channel(MerrySubChannel *channel)
{
    if (surelyF(channel == RET_NULL))
        return;
    merry_inactivate_channel(channel); // make sure everything is closed
    merry_destroy_pipe(channel->receive_pipe);
    merry_destroy_pipe(channel->send_pipe);
    free(channel);
}

mret_t merry_channel_read(MerrySubChannel *channel, mstr_t buf, msize_t buf_len)
{
    if (surelyF(channel == RET_NULL || buf == NULL))
        return RET_FAILURE;
    read(channel->receive_pipe->_read_fd, buf, buf_len); // we won't check the return
    return RET_SUCCESS;
}

mret_t merry_channel_write(MerrySubChannel *channel, mstr_t buf, msize_t buf_len)
{
    if (surelyF(channel == RET_NULL || buf == NULL))
        return RET_FAILURE;
    write(channel->send_pipe->_write_fd, buf, buf_len); // we won't check the return
    return RET_SUCCESS;
}

mret_t merry_reactivate_channel(MerrySubChannel *channel)
{
    if (surelyF(channel == RET_NULL))
        return RET_FAILURE;
    merry_inactivate_channel(channel); // make sure any fd was not open
    if (merry_pipe_reopen(channel->receive_pipe) == RET_FAILURE)
        return RET_FAILURE;
    if (merry_pipe_reopen(channel->send_pipe) == RET_FAILURE)
    {
        merry_pipe_close_both_ends(channel->receive_pipe);
        return RET_FAILURE;
    }
    channel->comms_active = mtrue;
    return RET_SUCCESS;
}

void merry_config_channel(MerrySubChannel *channel)
{
    // this call is done after the subsystem starts running
    // configure to make sure the right fd remain open
    if (surelyF(channel == RET_NULL))
        return RET_FAILURE;
    merry_pipe_close_one_end(channel->receive_pipe, _MERRY_CLOWEND_);
    merry_pipe_close_one_end(channel->send_pipe, _MERRY_CLOREND_);
    channel->comms_active = mtrue;
}