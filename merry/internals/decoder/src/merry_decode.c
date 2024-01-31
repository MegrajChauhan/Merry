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
    if ((merry_inst_queue_pop_instruction(decoder->queue, &decoder->core->ir)) != mtrue)
    {
        // the buffer is empty so we should wait for the buffer to be filled
        _llog_(_DECODER_, "INST_QEMPTY", "Inst queue is empty; Core ID %lu is sleeping", decoder->core->core_id);
        merry_cond_wait(decoder->core->cond, decoder->queue_lock);
        // when we are awakened again, we should get another instruction and this time it should work
        _llog_(_DECODER_, "INST_NEMPTY", "Inst queue populated; Core ID %lu waking up", decoder->core->core_id);
        merry_inst_queue_pop_instruction(decoder->queue, &decoder->core->ir);
    }
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

_MERRY_INTERNAL_ mqword_t merry_decoder_get_immediate(MerryDecoder *decoder)
{
    mqword_t res = 0;
    decoder->core->pc += 8;
    if (merry_memory_read_lock(decoder->core->inst_mem, decoder->core->pc, &res) == RET_FAILURE)
    {
        // we failed
        _llog_(_DECODER_, "DECODE_FAILED", "Decoding failed; Memory read failed", decoder->core->core_id);
        merry_requestHdlr_panic(decoder->core->inst_mem->error);
        merry_mutex_lock(decoder->core->lock);
        decoder->core->stop_running = mtrue; // stop further execution
        merry_mutex_unlock(decoder->core->lock);
    }
    return res;
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
                goto _next_;
            case OP_HALT: // Simply stop the core
                current_inst.exec_func = &merry_execute_halt;
                temp = mtrue; // for now
                break;        // halt has been broken down
            // Please ignore all of the redundant code
            /// TODO: Remove all these redundant code
            case OP_ADD_IMM:
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // get the immediate value
                current_inst.exec_func = &merry_execute_add_imm;
                break;
            case OP_ADD_REG:
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_add_reg;
                break;
            case OP_SUB_IMM:
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // get the immediate value
                current_inst.exec_func = &merry_execute_sub_imm;
                break;
            case OP_SUB_REG:
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_sub_reg;
                break;
            case OP_MUL_IMM:
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // get the immediate value
                current_inst.exec_func = &merry_execute_mul_imm;
                break;
            case OP_MUL_REG:
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_mul_reg;
                break;
            case OP_DIV_IMM:
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // get the immediate value
                current_inst.exec_func = &merry_execute_div_imm;
                break;
            case OP_DIV_REG:
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_div_reg;
                break;
            case OP_MOD_IMM:
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // get the immediate value
                current_inst.exec_func = &merry_execute_mod_imm;
                break;
            case OP_MOD_REG:
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_mod_reg;
                break;
            case OP_IADD_IMM:
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // get the immediate value
                current_inst.exec_func = &merry_execute_iadd_imm;
                break;
            case OP_IADD_REG:
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_iadd_reg;
                break;
            case OP_ISUB_IMM:
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // get the immediate value
                current_inst.exec_func = &merry_execute_isub_imm;
                break;
            case OP_ISUB_REG:
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_isub_reg;
                break;
            case OP_IMUL_IMM:
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // get the immediate value
                current_inst.exec_func = &merry_execute_imul_imm;
                break;
            case OP_IMUL_REG:
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_imul_reg;
                break;
            case OP_IDIV_IMM:
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // get the immediate value
                current_inst.exec_func = &merry_execute_idiv_imm;
                break;
            case OP_IDIV_REG:
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_idiv_reg;
                break;
            case OP_IMOD_IMM:
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // get the immediate value
                current_inst.exec_func = &merry_execute_imod_imm;
                break;
            case OP_IMOD_REG:
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_imod_reg;
                break;
            case OP_MOVE_IMM:                              // just 32 bits immediates
                current_inst.op1 = (current >> 40) & 15;   // get the destination register
                current_inst.op2 = (current) & 0xFFFFFFFF; // the immediate value
                current_inst.exec_func = &merry_execute_move_imm;
                break;
            case OP_MOVE_IMM_64:                                         // 64 bits immediates
                current_inst.op1 = (current >> 40) & 15;                 // get the destination register
                current_inst.op2 = merry_decoder_get_immediate(decoder); // get the next immediate
                current_inst.exec_func = &merry_execute_move_imm;
                break;
            case OP_MOVE_REG:                            // moves the whole 8 bytes
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_move_reg;
                break;
            case OP_MOVE_REG8:                           // moves only the lowest byte
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_move_reg8;
                break;
            case OP_MOVE_REG16:                          // moves only the lowest 2 bytes
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_move_reg16;
                break;
            case OP_MOVE_REG32:                          // moves only the lowest 4 byte
                current_inst.op1 = (current >> 44) & 15; // get the destination register which is also an operand
                current_inst.op2 = (current >> 40) & 15; // get the source register
                current_inst.exec_func = &merry_execute_move_reg32;
                break;
            case OP_MOVESX_IMM8:
                current_inst.op1 = (current >> 44) & 15;
                current_inst.op2 = (current) & 0xFF;
                current_inst.exec_func = &merry_execute_movesx_imm8;
                break;
            case OP_MOVESX_IMM16:
                current_inst.op1 = (current >> 44) & 15;
                current_inst.op2 = (current) & 0xFFFF;
                current_inst.exec_func = &merry_execute_movesx_imm16;
                break;
            case OP_MOVESX_IMM32:
                current_inst.op1 = (current >> 44) & 15;
                current_inst.op2 = (current) & 0xFFFFFF;
                current_inst.exec_func = &merry_execute_movesx_imm32;
                break;
            case OP_MOVESX_REG8:
                current_inst.op1 = (current >> 44) & 15;
                current_inst.op2 = (current >> 40) & 15;
                current_inst.exec_func = &merry_execute_movesx_reg8;
                break;
            case OP_MOVESX_REG16:
                current_inst.op1 = (current >> 44) & 15;
                current_inst.op2 = (current >> 40) & 15;
                current_inst.exec_func = &merry_execute_movesx_reg16;
                break;
            case OP_MOVESX_REG32:
                current_inst.op1 = (current >> 44) & 15;
                current_inst.op2 = (current >> 40) & 15;
                current_inst.exec_func = &merry_execute_movesx_reg32;
                break;
            case OP_JMP_OFF: // we have to make the 5 bytes 8 bytes by sign extension in case it is indeed in 2's complement
                // the offset in this case is a signed number and the 40th bit is the sign bit
                // the decoder executes the jump instruction
                core->pc = ((current & 0xFFFFFFFFFF) | 0xFFFFFF0000000000) - 8;
                goto _next_;
            case OP_JMP_ADDR:
                core->pc = (current & 0xFFFFFFFFFF) - 8;
                goto _next_;
                
            }
            merry_decoder_push_inst(decoder, &current_inst);
            goto _next_;
        _next_:
            core->pc += 8;
        }
    }
}