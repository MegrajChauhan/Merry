#include "../include/merry_cbuffer.h"

MerryCBuffer *merry_init_cbuffer(mcstr_t _dump_to_)
{
    MerryCBuffer *buf = (MerryCBuffer *)malloc(sizeof(MerryCBuffer));
    if (buf == NULL)
        return RET_NULL;
    buf->_dump_to_ = _dump_to_ != NULL ? _dump_to_ : NULL; // it can be NULL as well
    _MERRY_SQUEUE_INIT_(buf->queue)
    if (buf->queue == NULL)
    {
        free(buf);
        return RET_NULL;
    }
    _MERRY_SQUEUE_POPULATE_(buf->queue, _MERRY_CBUFFER_LEN_, char)
    if (buf->queue == NULL)
    {
        free(buf->queue);
        free(buf);
        return RET_NULL;
    }
    return buf;
}

void merry_destroy_cbuffer(MerryCBuffer *buf)
{
    if (buf == NULL)
        return;
    _MERRY_SQUEUE_DESTROY_(buf->queue)
    free(buf);
}

void merry_cbuffer_write(MerryCBuffer *buffer, mcstr_t _to_write_)
{
    if (_MERRY_IS_SQUEUE_FULL_(buffer->queue))
        return; // it is full
    // _to_write_ will certainly not be NULL
    register mqword_t _slen = strlen(_to_write_);
    register mqword_t _bufEmp = buffer->queue->qlen - buffer->queue->on_queue;
    // now we can only copy as much as the buffer can handle
    register mbool_t _will_be_full_ = _bufEmp > _slen ? mfalse : mtrue;
    memcpy(buffer->queue->queue + buffer->queue->on_queue, _to_write_, _will_be_full_ == mtrue ? _bufEmp : _slen); // this shouldn't fail
    if (_will_be_full_ == mtrue)
    {
        buffer->queue->tail += (_bufEmp); // update the tail
    }
    else
    {
        buffer->queue->tail += (_slen); // update the tail
    }
}

void merry_cbuffer_write_byte(MerryCBuffer *buffer, char _to_write_)
{
    if (_MERRY_IS_SQUEUE_FULL_(buffer->queue))
        return;           // it is full
    register mbool_t ret; // this doesn't really matter
    _MERRY_SQUEUE_PUSH_(buffer->queue, _to_write_, ret)
}

mbool_t merry_cbuffer_read_byte(MerryCBuffer *buffer, mstr_t _store_in)
{
    // the mfalse return will just indicate empty buffer
    register mbool_t ret = mtrue;
    _MERRY_SQUEUE_POP_(buffer->queue, *_store_in, ret)
    return ret;
}