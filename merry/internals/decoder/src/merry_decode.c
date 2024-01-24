#include "../merry_decode.h"

MerryDecoder *merry_init_decoder(MerryCore *host)
{
    MerryDecoder *decoder = (MerryDecoder *)malloc(sizeof(MerryDecoder));
    if (decoder == NULL)
        return RET_NULL;
    decoder->core = host;
    if ((decoder->lock = merry_mutex_init()) == NULL)
    {
        free(decoder);
        return RET_NULL;
    }
    if ((decoder->queue_lock = merry_mutex_init()) == NULL)
    {
        merry_mutex_destroy(decoder->lock);
        free(decoder);
        return RET_NULL;
    }
    if ((decoder->cond = merry_cond_init()) == NULL)
    {
        merry_mutex_destroy(decoder->lock);
        merry_mutex_destroy(decoder->queue_lock);
        free(decoder);
    }
    if ((decoder->queue = merry_inst_queue_init(_MERRY_INST_BUFFER_LEN_)) == RET_NULL)
    {
        merry_mutex_destroy(decoder->lock);
        merry_mutex_destroy(decoder->queue_lock);
        merry_cond_destroy(decoder->cond);
        free(decoder);
        return RET_NULL;
    }
    decoder->should_stop = mfalse;
    decoder->provide = mtrue;
    return decoder; // success
}

void merry_destroy_decoder(MerryDecoder *decoder)
{
    merry_mutex_destroy(decoder->lock);
    merry_inst_queue_destroy(decoder->queue);
    merry_cond_destroy(decoder->cond);
    merry_mutex_destroy(decoder->queue_lock);
    free(decoder);
}

// core's use this to get the next instruction
void merry_decoder_get_inst(MerryDecoder *decoder)
{
    // get an instruction from the Instruction queue and assign it to the core's IR register
    merry_mutex_lock(decoder->queue_lock);
    if ((merry_inst_queue_pop_instruction(decoder->queue, &decoder->core->ir)) != mtrue)
    {
        // the buffer is empty so we should wait for the buffer to be filled
        merry_cond_wait(decoder->core->cond, decoder->queue_lock);
        // when we are awakened again, we should get another instruction and this time it should work
        merry_inst_queue_pop_instruction(decoder->queue, &decoder->core->ir);
    }
    else
    {
        // if we got an instruction then see if the decoder had been waiting because the queue was full
        merry_cond_signal(decoder->cond); // this should work
    }
    merry_mutex_unlock(decoder->queue_lock);
}

mptr_t merry_decode(mptr_t d)
{
    MerryDecoder *decoder = (MerryDecoder *)d;
    MerryCore *core = decoder->core;
    MerryInstruction current_inst;
    mqword_t current;
    while (mtrue)
    {
        merry_mutex_lock(decoder->lock);
        if (decoder->should_stop == mtrue)
            break; // break out the loop [The core commands the decoder to stop]
        merry_mutex_unlock(decoder->lock);
        // this is where we decode the instructions
        // in case of error, the decoder will awake the core if it is sleeping then it will stop from furthur execution while the core receives instruction to stop and cleans up the decoder
        if (merry_manager_mem_read_inst(core->inst_mem, core->pc, &current) == RET_FAILURE)
        {
            // we failed
            merry_requestHdlr_panic(core->inst_mem->error);
            /// TODO: Replace all of these types of statements with atomic operations instead.
            merry_mutex_lock(core->lock);
            core->stop_running = mtrue; // stop further execution
            merry_mutex_unlock(core->lock);
            break; // stay out of it
        }
        // we have the instruction and we need to break it down to plump
        // Take the opcode first
        switch (merry_get_opcode(current))
        {
        case OP_NOP:  // we don't care about NOP instructions
        case OP_HALT: // Simply stop the core
           current_inst.opcode = OP_HALT;
           
        }
    }
}