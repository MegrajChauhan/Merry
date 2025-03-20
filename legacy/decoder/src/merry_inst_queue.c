#include "../merry_inst_queue.h"

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

void merry_inst_queue_destroy(MerryInstQueue *queue){
    _MERRY_DESTROY_QUEUE_NOPTR_(queue)}

mbool_t merry_inst_queue_push_instruction(MerryInstQueue *queue, MerryInstruction inst)
{
    mbool_t ret = mtrue;
    _MERRY_QUEUE_PUSH_NOPTR_(queue, inst, ret)
    return ret;
}

mbool_t merry_inst_queue_pop_instruction(MerryInstQueue *queue, MerryInstruction *inst)
{
    mbool_t ret = mtrue;
    _MERRY_QUEUE_POP_NOPTR_(queue, *inst, ret)
    return ret;
}

void merry_inst_queue_set_stack(MerryStack *stack)
{
    d_stack = stack;
}

void merry_inst_queue_hazard(MerryInstQueue *queue, MerryInstruction *inst)
{
    // set all the instructions after this instruction as NOP
    MerryInstQueueNode *next = (MerryInstQueueNode *)((mbptr_t)inst + sizeof(MerryInstruction));
    while (next != queue->tail)
    {
        if (next->value.exec_func == &merry_execute_call)
        {
            // if we have decoded any call instructions
            merry_stack_popn(d_stack); // we don't care about the order of the pop since there will be as many pops as there are call instructions
        }
        next->value.exec_func = &merry_execute_nop;
        next = next->next;
    }
    // since after reaching the tail we won't update it
    next->value.exec_func = &merry_execute_nop;
}

MerryInstruction *merry_inst_queue_get_next_tail(MerryInstQueue *queue)
{
    // return the next tail
    return &queue->tail->value;
}