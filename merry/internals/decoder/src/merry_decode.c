#include "../merry_decode.h"
#include "../../merry_core.h"

MerryDecoder *merry_init_decoder(MerryCore *host)
{
    _llog_(_DECODER_, "INIT", "Initializing decoder for core ID %lu", host->core_id);
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
    _llog_(_DECODER_, "SUCCESS", "Decoder init succeeded for core ID %lu", host->core_id);
    return decoder; // success
}

void merry_destroy_decoder(MerryDecoder *decoder)
{
    _llog_(_DECODER_, "DESTORY", "Destroying decoder for core ID %lu", decoder->core->core_id);
    merry_mutex_destroy(decoder->lock);
    merry_inst_queue_destroy(decoder->queue);
    merry_cond_destroy(decoder->cond);
    merry_mutex_destroy(decoder->queue_lock);
    free(decoder);
}

// cores use this to get the next instruction
void merry_decoder_get_inst(MerryDecoder *decoder)
{
    // get an instruction from the Instruction queue and assign it to the core's IR register
    _llog_(_DECODER_, "INST_REQ", "Core ID %lu requesting instruction", decoder->core->core_id);
    merry_mutex_lock(decoder->queue_lock);
    mbool_t x = merry_inst_queue_pop_instruction(decoder->queue, decoder->core->ir);
    printf("%lu\n", x);
    if ((x) != mtrue)
    {
        // the buffer is empty so we should wait for the buffer to be filled
        _llog_(_DECODER_, "INST_QEMPTY", "Inst queue is empty; Core ID %lu is sleeping", decoder->core->core_id);
        merry_cond_wait(decoder->core->cond, decoder->queue_lock);
        // when we are awakened again, we should get another instruction and this time it should work
        _llog_(_DECODER_, "INST_NEMPTY", "Inst queue populated; Core ID %lu waking up", decoder->core->core_id);
        printf("Getting inst: Waking up\n");
        merry_inst_queue_pop_instruction(decoder->queue, decoder->core->ir);
    }
    printf("Getting inst: Got %lu\n", decoder->core->ir->opcode);
    // if we got an instruction then see if the decoder had been waiting because the queue was full
    merry_cond_signal(decoder->cond); // this should work
    merry_mutex_unlock(decoder->queue_lock);
}

_MERRY_INTERNAL_ void merry_decoder_push_inst(MerryDecoder *decoder, MerryInstruction *inst)
{
    merry_mutex_lock(decoder->queue_lock);
    _llog_(_DECODER_, "PUSH_INST", "Decoder pushing inst; core ID %lu", decoder->core->core_id);
    if (merry_inst_queue_push_instruction(decoder->queue, *inst) == RET_FAILURE)
    {
        // the queue is full so wait until another instruction is popped
        _llog_(_DECODER_, "PUSH_INST_WAIT", "Inst queue full, sleeping; core ID %lu", decoder->core->core_id);
        merry_cond_wait(decoder->cond, decoder->queue_lock);
        // since we have been waiting for the signal
        merry_inst_queue_push_instruction(decoder->queue, *inst); // this should work unless the core has awakened us to exit
    }
    else
    {
        // we succeeded and so if the core had been waiting, wake it up
        if (decoder->queue->data_count == 1)
            merry_cond_signal(decoder->core->cond);
    }
    merry_mutex_unlock(decoder->queue_lock);
}

mptr_t merry_decode(mptr_t d)
{
    MerryDecoder *decoder = (MerryDecoder *)d;
    _llog_(_DECODER_, "RUNNING", "Decoder for core ID %lu running", decoder->core->core_id);
    MerryCore *core = decoder->core;
    MerryInstruction current_inst;
    mqword_t current;
    mbool_t temp = mfalse; // a temporary variable
    _llog_(_DECODER_, "STARTING", "Decoder starting decoding loop; core ID %lu", decoder->core->core_id);
    while (mtrue)
    {
        merry_mutex_lock(decoder->lock);
        if (decoder->should_stop == mtrue)
        {
            _llog_(_DECODER_, "STOPPING", "Decoder instructed to stop; core ID %Lu", decoder->core->core_id);
            break; // break out the loop [The core commands the decoder to stop]
        }
        merry_mutex_unlock(decoder->lock);
        // this is where we decode the instructions
        // in case of error, the decoder will awake the core if it is sleeping then it will stop from furthur execution while the core receives instruction to stop and cleans up the decoder
        if (merry_memory_read_lock(core->inst_mem, core->pc, &current) == RET_FAILURE)
        {
            // we failed
            _llog_(_DECODER_, "DECODE_FAILED", "Decoding failed; Memory read failed", decoder->core->core_id);
            merry_requestHdlr_panic(core->inst_mem->error);
            /// TODO: Replace all of these types of statements with atomic operations instead.
            merry_mutex_lock(core->lock);
            core->stop_running = mtrue; // stop further execution
            merry_mutex_unlock(core->lock);
            break; // stay out of it
        }
        // we have the instruction and we need to break it down to plump
        // Take the opcode first
        if (temp == mfalse)
        {
            switch (merry_get_opcode(current))
            {
            case OP_NOP: // we don't care about NOP instructions
                break;
            case OP_HALT:                      // Simply stop the core
                current_inst.opcode = OP_HALT; /*The halt instruction should be where we should stop furthur decoding but we still have to consider for branching and branch predictions as well */
                current_inst.exec_func = &merry_execute_halt;
                temp = mtrue; // for now
                merry_decoder_push_inst(decoder, &current_inst);
                break; // halt has been broken down
            }
            core->pc += 8;
            printf("%lu\n", core->pc);
        }
    }
}