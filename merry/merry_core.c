#include "internals/merry_core.h"
#include "internals/merry_os.h"

MerryCore *merry_core_init(MerryMemory *inst_mem, MerryMemory *data_mem, msize_t id)
{
    // allocate a new core
    _llog_(_CORE_, "Intialization", "Intializing core with ID %lu", id);
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
        goto failure;
    merry_core_zero_out_reg(new_core);
    new_core->stack_mem = (mqptr_t)_MERRY_MEM_GET_PAGE_(_MERRY_STACKMEM_BYTE_LEN_, _MERRY_PROT_DEFAULT_, _MERRY_FLAG_DEFAULT_);
    if (new_core->stack_mem == RET_NULL)
        goto failure;
    // new_core->should_wait = mtrue;   // should initially wait until said to run
    new_core->stop_running = mfalse; // this is set to false because as soon as the core is instructed to start/continue execution, it shouldn't stop and start immediately
    new_core->_is_private = mfalse;  // set to false by default
    // new_core->decoder = merry_init_decoder(new_core);
    // if (new_core->decoder == RET_NULL)
    //     goto failure;
    // if ((new_core->decoder_thread = merry_thread_init()) == RET_NULL)
    //     goto failure;
    // // we have done everything now
    new_core->ras = merry_init_stack(_MERRY_RAS_LEN_, mtrue, _MERRY_RAS_LIMIT_, _MERRY_RAS_GROW_PER_RESIZE_);
    if (new_core->ras == RET_NULL)
        goto failure;
    return new_core;
failure:
    _log_(_CORE_, "FAILURE", "Core intialization failed");
    merry_core_destroy(new_core);
    return RET_NULL;
}

void merry_core_destroy(MerryCore *core)
{
    _llog_(_CORE_, "DESTROYING", "Destroying core with ID %lu", core->core_id);
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
    // if (surelyT(core->decoder != NULL))
    // {
    //     merry_destroy_decoder(core->decoder);
    // }
    // if (surelyT(core->decoder_thread != NULL))
    // {
    //     merry_thread_destroy(core->decoder_thread);
    // }
    merry_destroy_stack(core->ras);
    core->data_mem = NULL;
    core->inst_mem = NULL;
    free(core);
}

_MERRY_INTERNAL_ mqword_t merry_core_get_immediate(MerryCore *core)
{
    mqword_t res = 0;
    core->pc += 8;
    if (merry_manager_mem_read_inst(core->inst_mem, core->pc, &res) == RET_FAILURE)
    {
        // we failed
        _llog_(_DECODER_, "DECODE_FAILED", "Decoding failed; Memory read failed", core->core_id);
        merry_requestHdlr_panic(core->inst_mem->error);
        merry_mutex_lock(core->lock);
        core->stop_running = mtrue; // stop further execution
        merry_mutex_unlock(core->lock);
    }
    return res;
}

mptr_t merry_runCore(mptr_t core)
{
    MerryCore *c = (MerryCore *)core;
    MerryInstruction *current_inst = &c->ir;
    mqword_t current = 0;
    while (mtrue)
    {
        merry_mutex_lock(c->lock);
        if (c->stop_running == mtrue)
        {
            _llog_(_CORE_, "STOPPING", "Core ID %lu stopping now", c->core_id);
            break;
        }
        merry_mutex_unlock(c->lock);
        if (merry_manager_mem_read_inst(c->inst_mem, c->pc, &current) == RET_FAILURE)
        {
            // we failed
            _llog_(_DECODER_, "DECODE_FAILED", "Decoding failed; Memory read failed", c->core_id);
            merry_requestHdlr_panic(c->inst_mem->error);
            /// TODO: Replace all of these types of statements with atomic operations instead.
            break; // stay out of it
        }
        switch (merry_get_opcode(current))
        {
        case OP_NOP: // we don't care about NOP instructions
            goto _next_;
        case OP_HALT: // Simply stop the core
            current_inst->exec_func = &merry_execute_halt;
            break; // halt has been broken down
        // Please ignore all of the redundant code
        /// TODO: Remove all these redundant code
        case OP_ADD_IMM:
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // get the immediate value
            current_inst->exec_func = &merry_execute_add_imm;
            break;
        case OP_ADD_REG:
            current_inst->op1 = (current >> 52) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current >> 48) & 15; // get the source register
            current_inst->exec_func = &merry_execute_add_reg;
            break;
        case OP_SUB_IMM:
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // get the immediate value
            current_inst->exec_func = &merry_execute_sub_imm;
            break;
        case OP_SUB_REG:
            current_inst->op1 = (current >> 52) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current >> 48) & 15; // get the source register
            current_inst->exec_func = &merry_execute_sub_reg;
            break;
        case OP_MUL_IMM:
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // get the immediate value
            current_inst->exec_func = &merry_execute_mul_imm;
            break;
        case OP_MUL_REG:
            current_inst->op1 = (current >> 52) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current >> 48) & 15; // get the source register
            current_inst->exec_func = &merry_execute_mul_reg;
            break;
        case OP_DIV_IMM:
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // get the immediate value
            current_inst->exec_func = &merry_execute_div_imm;
            break;
        case OP_DIV_REG:
            current_inst->op1 = (current >> 52) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current >> 48) & 15; // get the source register
            current_inst->exec_func = &merry_execute_div_reg;
            break;
        case OP_MOD_IMM:
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // get the immediate value
            current_inst->exec_func = &merry_execute_mod_imm;
            break;
        case OP_MOD_REG:
            current_inst->op1 = (current >> 52) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current >> 48) & 15; // get the source register
            current_inst->exec_func = &merry_execute_mod_reg;
            break;
        case OP_IADD_IMM:
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // get the immediate value
            current_inst->exec_func = &merry_execute_iadd_imm;
            break;
        case OP_IADD_REG:
            current_inst->op1 = (current >> 52) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current >> 48) & 15; // get the source register
            current_inst->exec_func = &merry_execute_iadd_reg;
            break;
        case OP_ISUB_IMM:
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // get the immediate value
            current_inst->exec_func = &merry_execute_isub_imm;
            break;
        case OP_ISUB_REG:
            current_inst->op1 = (current >> 52) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current >> 48) & 15; // get the source register
            current_inst->exec_func = &merry_execute_isub_reg;
            break;
        case OP_IMUL_IMM:
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // get the immediate value
            current_inst->exec_func = &merry_execute_imul_imm;
            break;
        case OP_IMUL_REG:
            current_inst->op1 = (current >> 52) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current >> 48) & 15; // get the source register
            current_inst->exec_func = &merry_execute_imul_reg;
            break;
        case OP_IDIV_IMM:
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // get the immediate value
            current_inst->exec_func = &merry_execute_idiv_imm;
            break;
        case OP_IDIV_REG:
            current_inst->op1 = (current >> 52) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current >> 48) & 15; // get the source register
            current_inst->exec_func = &merry_execute_idiv_reg;
            break;
        case OP_IMOD_IMM:
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // get the immediate value
            current_inst->exec_func = &merry_execute_imod_imm;
            break;
        case OP_IMOD_REG:
            current_inst->op1 = (current >> 52) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current >> 48) & 15; // get the source register
            current_inst->exec_func = &merry_execute_imod_reg;
            break;
        case OP_MOVE_IMM:                               // just 32 bits immediates
            current_inst->op1 = (current >> 48) & 15;   // get the destination register
            current_inst->op2 = (current) & 0xFFFFFFFF; // the immediate value
            current_inst->exec_func = &merry_execute_move_imm;
            break;
        case OP_MOVE_IMM_64:                               // 64 bits immediates
            current_inst->op1 = (current) & 15;            // get the destination register
            current_inst->op2 = merry_core_get_immediate(c); // get the next immediate
            current_inst->exec_func = &merry_execute_move_imm;
            break;
        case OP_MOVE_REG:                            // moves the whole 8 bytes
            current_inst->op1 = (current >> 4) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current) & 15;      // get the source register
            current_inst->exec_func = &merry_execute_move_reg;
            break;
        case OP_MOVE_REG8:                           // moves only the lowest byte
            current_inst->op1 = (current >> 4) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current) & 15;      // get the source register
            current_inst->exec_func = &merry_execute_move_reg8;
            break;
        case OP_MOVE_REG16:                          // moves only the lowest 2 bytes
            current_inst->op1 = (current >> 4) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current) & 15;      // get the source register
            current_inst->exec_func = &merry_execute_move_reg16;
            break;
        case OP_MOVE_REG32:                          // moves only the lowest 4 byte
            current_inst->op1 = (current >> 4) & 15; // get the destination register which is also an operand
            current_inst->op2 = (current) & 15;      // get the source register
            current_inst->exec_func = &merry_execute_move_reg32;
            break;
        case OP_MOVESX_IMM8:
            current_inst->op1 = (current >> 48) & 15;
            current_inst->op2 = (current) & 0xFF;
            current_inst->exec_func = &merry_execute_movesx_imm8;
            break;
        case OP_MOVESX_IMM16:
            current_inst->op1 = (current >> 48) & 15;
            current_inst->op2 = (current) & 0xFFFF;
            current_inst->exec_func = &merry_execute_movesx_imm16;
            break;
        case OP_MOVESX_IMM32:
            current_inst->op1 = (current >> 48) & 15;
            current_inst->op2 = (current) & 0xFFFFFF;
            current_inst->exec_func = &merry_execute_movesx_imm32;
            break;
        case OP_MOVESX_REG8:
            current_inst->op1 = (current >> 44) & 15;
            current_inst->op2 = (current >> 40) & 15;
            current_inst->exec_func = &merry_execute_movesx_reg8;
            break;
        case OP_MOVESX_REG16:
            current_inst->op1 = (current >> 44) & 15;
            current_inst->op2 = (current >> 40) & 15;
            current_inst->exec_func = &merry_execute_movesx_reg16;
            break;
        case OP_MOVESX_REG32:
            current_inst->op1 = (current >> 44) & 15;
            current_inst->op2 = (current >> 40) & 15;
            current_inst->exec_func = &merry_execute_movesx_reg32;
            break;
        case OP_JMP_OFF: // we have to make the 5 bytes 8 bytes by sign extension in case it is indeed in 2's complement
            // the offset in this case is a signed number and the 40th bit is the sign bit
            // the decoder executes the jump instruction
            mqword_t off = (current & 0xFFFFFFFFFFFF);
            if ((off >> 47) == 1)
                off |= 0xFFFF000000000000;
            c->pc += off - 8;
            goto _next_;
        case OP_JMP_ADDR:
            c->pc = merry_core_get_immediate(c) - 8;
            goto _next_;
        case OP_CALL:
            // save the current return address
            if (merry_stack_push(c->ras, c->pc) == RET_FAILURE)
            {
                merry_requestHdlr_panic(MERRY_CALL_DEPTH_REACHED);
                goto _next_;
            }
            c->pc = merry_core_get_immediate(c) - 8; // the address to the first instruction of the procedure
            current_inst->exec_func = &merry_execute_call;
            break;
        case OP_RET:
            // we just have to pop the topmost
            if (merry_stack_pop(c->ras, &c->pc) == RET_FAILURE)
            {
                merry_requestHdlr_panic(MERRY_INVALID_RETURN);
                goto _next_;
            }
            // pc should have been restored
            current_inst->exec_func = &merry_execute_ret;
            break;
        case OP_SVA: // [SVA stands for Stack Variable Access]
            current_inst->op1 = (current >> 48) & 15;
            current_inst->op2 = (current) & 0xFFFF; // the offset can only be 2 bytes long
            current_inst->exec_func = &merry_execute_sva;
            break;
        case OP_SVC: // [SVC stands for Stack Variable Change]
            current_inst->op1 = (current >> 48) & 15;
            current_inst->op2 = (current) & 0xFFFF; // the offset can only be 2 bytes long
            current_inst->exec_func = &merry_execute_svc;
            break;
        case OP_PUSH_IMM:
            current_inst->exec_func = &merry_execute_push_imm;
            current_inst->op1 = current & 0xFFFFFFFFFFFF; // 6 bytes of immediates
            break;
        case OP_PUSH_REG:
            current_inst->exec_func = &merry_execute_push_reg;
            current_inst->op1 = current & 15; // the destination register
            break;
        case OP_POP:
            current_inst->exec_func = &merry_execute_pop;
            current_inst->op1 = current & 15;
            break;
        case OP_PUSHA:
            current_inst->exec_func = &merry_execute_pusha;
            break;
        case OP_POPA:
            current_inst->exec_func = &merry_execute_popa;
            break;
        case OP_AND_IMM:
            current_inst->exec_func = &merry_execute_and_imm;
            current_inst->op1 = (current) & 15;
            current_inst->op2 = merry_core_get_immediate(c);
            break;
        case OP_AND_REG:
            current_inst->exec_func = &merry_execute_and_reg;
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            break;
        case OP_OR_IMM:
            current_inst->exec_func = &merry_execute_or_imm;
            current_inst->op1 = (current) & 15;
            current_inst->op2 = merry_core_get_immediate(c);
            break;
        case OP_OR_REG:
            current_inst->exec_func = &merry_execute_or_imm;
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            break;
        case OP_XOR_IMM:
            current_inst->exec_func = &merry_execute_xor_imm;
            current_inst->op1 = (current) & 15;
            current_inst->op2 = merry_core_get_immediate(c);
            break;
        case OP_XOR_REG:
            current_inst->exec_func = &merry_execute_xor_reg;
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            break;
        case OP_NOT:
            current_inst->exec_func = &merry_execute_not;
            current_inst->op1 = current & 15;
            break;
        case OP_LSHIFT:
            current_inst->exec_func = &merry_execute_lshift;
            current_inst->op1 = (current >> 8) & 15;
            current_inst->op2 = current & 0x40;
            break;
        case OP_RSHIFT:
            current_inst->exec_func = &merry_execute_rshift;
            current_inst->op1 = (current >> 8) & 15;
            current_inst->op2 = current & 0x40;
            break;
        case OP_CMP_IMM:
            current_inst->op1 = current & 15;
            current_inst->op2 = merry_core_get_immediate(c);
            current_inst->exec_func = &merry_execute_cmp_imm;
            break;
        case OP_CMP_REG:
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            current_inst->exec_func = &merry_execute_cmp_reg;
            break;
        case OP_INC:
            current_inst->exec_func = &merry_execute_inc;
            current_inst->op1 = current & 15;
            break;
        case OP_DEC:
            current_inst->exec_func = &merry_execute_dec;
            current_inst->op1 = current & 15;
            break;
        case OP_LEA:
            // 000000000 0000000 00000000 00000000 00000000 00000000 00000000 00000000
            current_inst->exec_func = &merry_execute_lea;
            current_inst->op1 = (current >> 24) & 15;
            current_inst->op2 = (current >> 16) & 15;
            current_inst->Oop3 = (current >> 8) & 15;
            current_inst->flag = (current) & 15;
            break;
        case OP_LOAD:
            current_inst->exec_func = &merry_execute_load;
            current_inst->op1 = (current) & 15; // the destination register
            current_inst->op2 = merry_core_get_immediate(c);
            break;
        case OP_STORE:
            current_inst->exec_func = &merry_execute_store;
            current_inst->op1 = (current) & 15; // the destination register
            current_inst->op2 = merry_core_get_immediate(c);
            break;
        case OP_EXCG8:
            current_inst->exec_func = &merry_execute_excg8;
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            break;
        case OP_EXCG16:
            current_inst->exec_func = &merry_execute_excg16;
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            break;
        case OP_EXCG32:
            current_inst->exec_func = &merry_execute_excg32;
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            break;
        case OP_EXCG:
            current_inst->exec_func = &merry_execute_excg;
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            break;
        case OP_MOV8:
            current_inst->exec_func = &merry_execute_mov8;
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            break;
        case OP_MOV16:
            current_inst->exec_func = &merry_execute_mov16;
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            break;
        case OP_MOV32:
            current_inst->exec_func = &merry_execute_mov32;
            current_inst->op1 = (current >> 4) & 15;
            current_inst->op2 = current & 15;
            break;
        case OP_CFLAGS:
            current_inst->exec_func = &merry_execute_cflags;
            break;
        case OP_RESET:
            current_inst->exec_func = &merry_execute_reset;
            break;
        case OP_CLC:
            current_inst->exec_func = &merry_execute_clc;
            break;
        case OP_CLZ:
            current_inst->exec_func = &merry_execute_clz;
            break;
        case OP_CLN:
            current_inst->exec_func = &merry_execute_cln;
            break;
        case OP_CLO:
            current_inst->exec_func = &merry_execute_clo;
            break;
        case OP_JNZ:
            // the address to jmp should follow the instruction
            current_inst->exec_func = &merry_execute_jnz;
            current_inst->op1 = merry_core_get_immediate(c);
            break;
        }
        current_inst->exec_func(c);
        goto _next_;
    _next_:
        c->pc += 8;
    }
}
