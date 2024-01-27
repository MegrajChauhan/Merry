#include "../inst/merry_inst_queue.h"

MerryInstQueue *merry_inst_queue_init(msize_t queue_len)
{
    MerryInstQueue *queue = (MerryInstQueue *)malloc(sizeof(MerryInstQueue));
    if (queue == NULL)
        return NULL;
    mbool_t ret = mtrue;
    // populate the nodes
    _MERRY_QUEUE_CREATE_NODES_NOPTR_(queue, MerryInstQueueNode, queue_len, ret)
    if (ret == mfalse)
    {
        _MERRY_DESTROY_QUEUE_NOPTR_(queue)
        return RET_NULL;
    }
    return queue;
}

void merry_inst_queue_destroy(MerryInstQueue *queue)
{
    _MERRY_DESTROY_QUEUE_NOPTR_(queue)
}

mret_t merry_inst_queue_push_instruction(MerryInstQueue *queue, MerryInstruction inst)
{
    mbool_t ret = mtrue;
    _MERRY_QUEUE_PUSH_NOPTR_(queue, inst, ret)
    return ret;
}

mret_t merry_inst_queue_pop_instruction(MerryInstQueue *queue, MerryInstruction *inst)
{
    mbool_t ret = mtrue;
    _MERRY_QUEUE_POP_NOPTR_(queue, *inst, ret)
    return ret;
}