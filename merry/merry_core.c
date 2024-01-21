#include "internals/merry_core.h"

MerryCore *merry_core_init(MerryMemory *inst_mem, MerryMemory *data_mem, msize_t id)
{
    // allocate a new core
    MerryCore *new_core = (MerryCore *)malloc(sizeof(MerryCore));
    // check if the core has been initialized
    if (new_core == RET_NULL)
        return RET_NULL;
    new_core->bp = 0; // initialize these registers to 0
    new_core->pc = 0;
    new_core->sp = 0;
    new_core->core_id = id;
    new_core->data_mem = data_mem;
    new_core->inst_mem = inst_mem;
    // initialize the locks and condition variables
    new_core->cond = merry_cond_init();
    if (new_core->cond == RET_NULL)
    {
        free(new_core);
        return RET_NULL;
    }
    new_core->lock = merry_mutex_init();
    if (new_core->lock == RET_NULL)
    {
        merry_cond_destroy(new_core->cond);
        free(new_core);
        return RET_NULL;
    }
    new_core->registers = (mqptr_t)malloc(sizeof(mqword_t) * REGR_COUNT);
    if (new_core->registers == RET_NULL)
    {
        merry_core_destroy(new_core);
        return RET_NULL;
    }
    new_core->stack_mem = (mqptr_t)_MERRY_MEM_GET_PAGE_(_MERRY_STACKMEM_BYTE_LEN_, _MERRY_PROT_DEFAULT_, _MERRY_FLAG_DEFAULT_);
    if (new_core->stack_mem == RET_NULL)
    {
        merry_core_destroy(new_core);
        return RET_NULL;
    }
    // new_core->should_wait = mtrue;   // should initially wait until said to run
    new_core->stop_running = mfalse; // this is set to false because as soon as the core is instructed to start/continue execution, it shouldn't stop and start immediately
    new_core->_is_private = mfalse;  // set to false by default
    // we have done everything now
    return new_core;
}

void merry_core_destroy(MerryCore *core)
{
    if (surelyF(core == NULL))
        return;
    if (surelyT(core->cond != NULL))
    {
        merry_cond_destroy(core->cond);
    }
    if (surelyT(core->lock != NULL))
    {
        merry_mutex_destroy(core->lock);
    }
    if (surelyT(core->registers != NULL))
    {
        free(core->registers);
    }
    if (surelyT(core->stack_mem != NULL))
    {
        if (_MERRY_MEM_GIVE_PAGE_(core->stack_mem, _MERRY_STACKMEM_BYTE_LEN_) == _MERRY_RET_GIVE_ERROR_)
        {
            // we failed here
            // This shouldn't happen unless the stack_mem variable was messed with and corrupted
            // This is the worst idea but we simply do nothing[OPPS UNSAFE? PFTT...WHAT COULD POSSIBLY GO WRONG?!]
            // nothing really, after the VM terminates, the OS will unmap all the resources that was being used and hence we becomes safe
            // While it would be interesting to know why this error happened, the cause probably didn't come from the VM itself since i am trying to write it as "SAFE" as possible
            // This may sound like a joke considering how absurd, unstructured, redundand, unsafe the code so far has been
        }
    }
    core->data_mem = NULL;
    core->inst_mem = NULL;
    free(core);
}

mptr_t merry_runCore(mptr_t core)
{
    MerryCore *c = (MerryCore *)core;
    // the core is now in action
    // it's internal's are all initialized and it is ready to go
    // it would be better to implement a decoder that runs in the background and prefetch and decode the instructions
    // While this would be better it comes with a lot of complexity and overheads and still providing a lot of awesome perks
    while (mtrue)
    {
        // merry_mutex_lock(c->lock);
        // if (c->stop_running == mtrue)
        //     break;
        // if ((merry_manager_mem_read_inst(c->inst_mem, c->pc, &c->ir)) == RET_FAILURE)
        // {
        //     // // failed to read
        //     // merry_os_handle_error(c->os, c->inst_mem->error);
        //     // break;
        //     // in the next cycle, this core will have stopped
        // }
        // merry_mutex_unlock(c->lock);
        // switch (_MERRY_CORE_GET_OPCODE_(c->ir))
        // {
        // case OP_NOP:
        //     break;
        // case OP_HALT: // halt instruction, will make this core stop executing at all
        //     c->stop_running = mtrue;
        //     break;
        // }
    }
    return RET_NULL; // return nothing
}