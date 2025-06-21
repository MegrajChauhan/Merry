#include "merry_64_bit_core.h"

MerryCoreBase *merry_64_bit_core_base(MerryState *state) {
  MerryCoreBase *base = (MerryCoreBase *)malloc(sizeof(MerryCoreBase));
  if (!base) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    merry_provide_context(*state, _MERRY_CORE_BASE_INITIALIZATION_);
    return RET_NULL;
  }
  /*
    if ((base->execution_queue = merry_dynamic_queue_init(state)) == RET_NULL) {
      merry_provide_context(*state, _MERRY_CORE_BASE_INITIALIZATION_);
      free(base);
      return RET_NULL;
    }
  */

  if ((base->execution_states = merry_create_dynamic_list(
           _MERRY_EXECUTING_STATE_INIT_CAP_, sizeof(Merry64BitCoreState),
           &base->state)) == RET_NULL) {
    merry_provide_context(*state, _MERRY_CORE_BASE_INITIALIZATION_);
    free(base);
    return RET_NULL;
  }

  base->init_func = merry_64_bit_core_init;
  base->free_func = merry_64_bit_core_destroy;
  base->exec_func = merry_64_bit_core_run;
  base->save_state_func = merry_64_bit_core_save_state;
  base->jmp_state_func = merry_64_bit_core_jmp_state;
  base->del_state_func = merry_64_bit_core_del_state;
  base->replace_state_func = merry_64_bit_core_replace_state;

  if (merry_cond_init(&base->cond) == RET_FAILURE) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OBTAIN_COND_);
    merry_provide_context(*state, _MERRY_CORE_BASE_INITIALIZATION_);
    //    merry_dynamic_queue_destroy(base->execution_queue);
    merry_destroy_dynamic_list(base->execution_states);
    free(base);
    return RET_NULL;
  }

  if (merry_mutex_init(&base->lock) == RET_FAILURE) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OBTAIN_LOCK_);
    merry_provide_context(*state, _MERRY_CORE_BASE_INITIALIZATION_);
    merry_cond_destroy(&base->cond);
    merry_destroy_dynamic_list(base->execution_states);
    //  merry_dynamic_queue_destroy(base->execution_queue);
    free(base);
    return RET_NULL;
  }

  if ((base->wild_request = merry_create_simple_queue(
           _MERRY_WILD_REQUEST_QUEUE_LEN_, sizeof(MerryWildRequest),
           &base->state)) == RET_NULL) {
    merry_provide_context(base->state, _MERRY_CORE_BASE_INITIALIZATION_);
    merry_cond_destroy(&base->cond);
    merry_mutex_destroy(&base->lock);
    merry_destroy_dynamic_list(base->execution_states);
    //  merry_dynamic_queue_destroy(base->execution_queue);
    free(base);
    return RET_NULL;
  }

  base->priviledge = mfalse;
  base->terminate = mfalse;
  base->terminate = mtrue;
  base->do_not_disturb = mfalse;
  base->ignore_pause = mfalse;
  base->wrequest = mfalse;
  base->pause = mfalse;
  base->stop = mfalse;
  base->permission_to_pause = mfalse;
  base->permission_to_unpause = mfalse;
  base->permission_to_add_mem_page = mfalse;
  base->permission_to_bestow_priviledge = mfalse;
  base->permission_to_change_parent = mfalse;
  base->permission_to_create_group = mfalse;
  base->permission_to_create_thread = mfalse;
  base->permission_to_kill_core = mfalse;
  base->wild_request_hdlr_set = mfalse;
  base->core_type = __CORE_64_BIT;
  merry_assign_state(base->state, _MERRY_ORIGIN_NONE_, 0);
  base->state.child_state = NULL;
  base->wild_request = 0;
  base->active_state = (mqword_t)(-1);
  return base;
}

void *merry_64_bit_core_init(MerryCoreBase *base, MerryRAM *ram, MerryRAM *iram,
                             maddress_t start_point) {
  merry_check_ptr(base);
  merry_check_ptr(ram);
  merry_check_ptr(iram);

  base->state.arg.qword = base->core_id;
  Merry64BitCore *core = (Merry64BitCore *)malloc(sizeof(Merry64BitCore));
  if (!core) {
    merry_assign_state(base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    merry_provide_context(base->state, _MERRY_CORE_INITIALIZATION_);
    return RET_NULL;
  }
  core->base = base;
  core->pc = start_point;
  core->base->ram = ram;
  core->base->iram = iram;
  core->regr[SP] = 0;
  core->regr[BP] = 0;

  MerryRAM *stack = merry_create_RAM(2, &base->state); // 2MB

  if (!stack) {
    merry_provide_context(base->state, _MERRY_CORE_INITIALIZATION_);
    free(core);
    return RET_NULL;
  }

  MerryStack *tbs = merry_stack_init(_MERRY_64_BIT_CORE_TBS_LIM_, &base->state);

  if (!tbs) {
    merry_provide_context(base->state, _MERRY_CORE_INITIALIZATION_);
    merry_destroy_RAM(stack);
    free(core);
    return RET_NULL;
  }

  MerryStack *cstack =
      merry_stack_init(_MERRY_64_BIT_CORE_TBS_LIM_, &base->state);

  if (!cstack) {
    merry_provide_context(base->state, _MERRY_CORE_INITIALIZATION_);
    merry_destroy_RAM(stack);
    merry_stack_destroy(tbs);
    free(core);
    return RET_NULL;
  }

  if ((core->req = (MerryGravesRequest *)malloc(sizeof(MerryGravesRequest))) ==
      NULL) {
    merry_assign_state(base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    merry_provide_context(base->state, _MERRY_CORE_INITIALIZATION_);
    merry_destroy_RAM(stack);
    merry_stack_destroy(tbs);
    merry_stack_destroy(cstack);
    free(core);
    return RET_NULL;
  }

  core->req->base = core->base;
  core->stack = stack;
  core->tbs = tbs;
  core->cstack = cstack;

  return (void *)core;
}

void merry_64_bit_core_destroy(void *cptr) {
  merry_check_ptr(cptr);
  Merry64BitCore *c = (Merry64BitCore *)cptr;
  merry_core_base_clean(c->base);

  merry_destroy_RAM(c->stack);
  merry_stack_destroy(c->tbs);
  merry_stack_destroy(c->cstack);
  // The RAM is handled by Graves

  free(c->req);
  free(cptr);
}

_THRET_T_ merry_64_bit_core_run(void *arg) {
  merry_check_ptr(arg);
  register Merry64BitCore *core = (Merry64BitCore *)arg;
  MerryHostMemLayout current;
  MerryHostMemLayout mem;
  MerryDoubleToQword dtoq;
  MerryFloatToDword ftod;

  while (mtrue) {
    if (core->base->stop) {
      merry_mutex_lock(&core->base->lock);
      if (core->base->terminate) {
        core->req->type = SHUT_DOWN;
        merry_SEND_REQUEST(core->req);
        merry_mutex_unlock(&core->base->lock);
        break;
      } else if (core->base->pause &&
                 (!core->base->ignore_pause || !core->base->do_not_disturb)) {
        merry_cond_wait(&core->base->cond, &core->base->lock);
        core->base->pause = mfalse;
      } else if (core->base->wrequest && !core->base->do_not_disturb) {
        if (!core->base->wild_request_hdlr_set) {
          // we ignore if no whdlr is provided
        } else {
          // save the current state and jump to
          if (merry_simple_queue_empty(core->base->wild_request)) {
            core->base->wrequest = mfalse;
          } else {
            if (merry_64_bit_core_save_state(arg) == RET_FAILURE) {
              core->req->type = PROBLEM_ENCOUNTERED;
              merry_SEND_REQUEST(core->req);
              core->req->type = SHUT_DOWN;
              merry_SEND_REQUEST(core->req);
              merry_mutex_unlock(&core->base->lock);
              break;
            }
            // It is the handler's job to restore the state
            MerryWildRequest tmp;
            merry_simple_queue_dequeue(core->base->wild_request, &tmp);
            core->regr[R0] = tmp.request;
            core->regr[R1] = tmp.requester_id;
            core->regr[R2] = tmp.requester_uid;
            core->regr[R3] = tmp.arg;
            core->pc = core->base->wild_request_hdlr;
            core->base->prior_active_state = core->base->active_state;
            core->base->active_state =
                merry_dynamic_list_size(core->base->execution_states);
          }
        }
      }
      core->base->pause = mfalse;
      core->base->stop = mfalse;
      merry_mutex_unlock(&core->base->lock);
    }
    if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                       core->base->iram, core->pc,
                                       &current.whole_word) == RET_FAILURE)
      break;
    switch (current.bytes.b0) {
    case OP_NOP:
      break;
    case OP_HALT:
      core->base->stop = mtrue;
      break;
    case OP_ADD_IMM:
      add_imm64(core, current);
      break;
    case OP_ADD_REG:
      add_reg64(core, current);
      break;
    case OP_SUB_IMM:
      sub_imm64(core, current);
      break;
    case OP_SUB_REG:
      sub_reg64(core, current);
      break;
    case OP_MUL_IMM:
      mul_imm64(core, current);
      break;
    case OP_MUL_REG:
      mul_reg64(core, current);
      break;
    case OP_DIV_IMM:
      div_imm64(core, current);
      break;
    case OP_DIV_REG:
      div_reg64(core, current);
      break;
    case OP_MOD_IMM:
      mod_imm64(core, current);
      break;
    case OP_MOD_REG:
      mod_reg64(core, current);
      break;
    case OP_IADD_IMM:
      iadd_imm64(core, current);
      break;
    case OP_IADD_REG:
      iadd_reg64(core, current);
      break;
    case OP_ISUB_IMM:
      isub_imm64(core, current);
      break;
    case OP_ISUB_REG:
      isub_reg64(core, current);
      break;
    case OP_IMUL_IMM:
      imul_imm64(core, current);
      break;
    case OP_IMUL_REG:
      imul_reg64(core, current);
      break;
    case OP_IDIV_IMM:
      idiv_imm64(core, current);
      break;
    case OP_IDIV_REG:
      idiv_reg64(core, current);
      break;
    case OP_IMOD_IMM:
      imod_imm64(core, current);
      break;
    case OP_IMOD_REG:
      imod_reg64(core, current);
      break;
    case OP_FADD:
      fadd64(core, current);
      break;
    case OP_FSUB:
      fsub64(core, current);
      break;
    case OP_FMUL:
      fmul64(core, current);
      break;
    case OP_FDIV:
      fdiv64(core, current);
      break;
    case OP_FADD32:
      fadd3264(core, current);
      break;
    case OP_FSUB32:
      fsub3264(core, current);
      break;
    case OP_FMUL32:
      fmul3264(core, current);
      break;
    case OP_FDIV32:
      fdiv3264(core, current);
      break;
    case OP_ADD_MEMB:
      add_memb64(core, current);
      break;
    case OP_ADD_MEMW:
      add_memw64(core, current);
      break;
    case OP_ADD_MEMD:
      add_memd64(core, current);
      break;
    case OP_ADD_MEMQ:
      add_memq64(core, current);
      break;
    case OP_SUB_MEMB:
      sub_memb64(core, current);
      break;
    case OP_SUB_MEMW:
      sub_memw64(core, current);
      break;
    case OP_SUB_MEMD:
      sub_memd64(core, current);
      break;
    case OP_SUB_MEMQ:
      sub_memq64(core, current);
      break;
    case OP_MUL_MEMB:
      mul_memb64(core, current);
      break;
    case OP_MUL_MEMW:
      mul_memw64(core, current);
      break;
    case OP_MUL_MEMD:
      mul_memd64(core, current);
      break;
    case OP_MUL_MEMQ:
      mul_memq64(core, current);
      break;
    case OP_DIV_MEMB:
      div_memb64(core, current);
      break;
    case OP_DIV_MEMW:
      div_memw64(core, current);
      break;
    case OP_DIV_MEMD:
      div_memd64(core, current);
      break;
    case OP_DIV_MEMQ:
      div_memq64(core, current);
      break;
    case OP_MOD_MEMB:
      mod_memb64(core, current);
      break;
    case OP_MOD_MEMW:
      mod_memw64(core, current);
      break;
    case OP_MOD_MEMD:
      mod_memd64(core, current);
      break;
    case OP_MOD_MEMQ:
      mod_memq64(core, current);
      break;
    case OP_FADD_MEM:
      fadd64_mem64(core, current);
      break;
    case OP_FSUB_MEM:
      fsub64_mem64(core, current);
      break;
    case OP_FMUL_MEM:
      fmul64_mem64(core, current);
      break;
    case OP_FDIV_MEM:
      fdiv64_mem64(core, current);
      break;
    case OP_FADD32_MEM:
      fadd32_mem64(core, current);
      break;
    case OP_FSUB32_MEM:
      fsub64_mem64(core, current);
      break;
    case OP_FMUL32_MEM:
      fmul32_mem64(core, current);
      break;
    case OP_FDIV32_MEM:
      fdiv32_mem64(core, current);
      break;
    case OP_INC:
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR]++;
      break;
    case OP_DEC:
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR]--;
      break;
    case OP_MOVE_IMM_64:
    case OP_MOVF:
      mov_imm64(core, current);
      break;
    case OP_MOVF32:
      movf3264(core, current);
      break;
    case OP_MOVE_REG:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] =
          core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      break;
    case OP_MOVE_REG8:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] =
          core->regr[current.bytes.b7 & GPC_64_LAST_REGR] & 255;
      break;
    case OP_MOVE_REG16:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] =
          core->regr[current.bytes.b7 & GPC_64_LAST_REGR] & 0xFFFF;
      break;
    case OP_MOVE_REG32:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] =
          core->regr[current.bytes.b7 & GPC_64_LAST_REGR] & 0xFFFFFFFF;
      break;
    case OP_MOVESX_IMM8:
      movesx_imm864(core, current);
      break;
    case OP_MOVESX_IMM16:
      movesx_imm1664(core, current);
      break;
    case OP_MOVESX_IMM32:
      movesx_imm3264(core, current);
      break;
    case OP_MOVESX_REG8:
      movesx_reg864(core, current);
      break;
    case OP_MOVESX_REG16:
      movesx_reg1664(core, current);
      break;
    case OP_MOVESX_REG32:
      movesx_reg3264(core, current);
      break;
    case OP_EXCG8:
      excg864(core, current);
      break;
    case OP_EXCG16:
      excg1664(core, current);
      break;
    case OP_EXCG32:
      excg3264(core, current);
      break;
    case OP_EXCG:
      excg64(core, current);
      break;
    case OP_MOV8:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] &=
          (0xFFFFFFFFFFFFFF00 |
           (core->regr[current.bytes.b7 & GPC_64_LAST_REGR] & 0xFF));
      break;
    case OP_MOV16:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] &=
          (0xFFFFFFFFFFFF0000 |
           (core->regr[current.bytes.b7 & GPC_64_LAST_REGR] & 0xFFFF));
      break;
    case OP_MOV32:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] &=
          (0xFFFFFFFF00000000 |
           (core->regr[current.bytes.b7 & GPC_64_LAST_REGR] & 0xFFFFFFFF));
      break;
    case OP_MOVNZ:
    case OP_MOVNE:
      if (core->fregr.zero == 0)
        mov_imm64(core, current);
      break;
    case OP_MOVE:
    case OP_MOVZ:
      if (core->fregr.zero == 1)
        mov_imm64(core, current);
      break;
    case OP_MOVNC:
      if (core->fregr.carry == 0)
        mov_imm64(core, current);
      break;
    case OP_MOVC:
      if (core->fregr.carry == 1)
        mov_imm64(core, current);
      break;
    case OP_MOVNO:
      if (core->fregr.overflow == 0)
        mov_imm64(core, current);
      break;
    case OP_MOVO:
      if (core->fregr.overflow == 1)
        mov_imm64(core, current);
      break;
    case OP_MOVNN:
      if (core->fregr.negative == 0)
        mov_imm64(core, current);
      break;
    case OP_MOVN:
      if (core->fregr.negative == 1)
        mov_imm64(core, current);
      break;
    case OP_MOVS:
    case OP_MOVNG:
      if (core->fregr.negative == 1)
        mov_imm64(core, current);
      break;
    case OP_MOVNS:
    case OP_MOVG:
      if (core->fregr.negative == 0)
        mov_imm64(core, current);
      break;
    case OP_MOVGE:
      if (core->fregr.negative == 0 || core->fregr.zero == 1)
        mov_imm64(core, current);
      break;
    case OP_MOVSE:
      if (core->fregr.zero == 1 || core->fregr.negative == 1)
        mov_imm64(core, current);
      break;
    case OP_JMP_OFF:
      sign_extend32(current.half_words.w1);
      core->pc += current.half_words.w1;
      break;
    case OP_JMP_ADDR:
      core->pc = current.whole_word & 0xFFFFFFFFFFFF;
      break;
    case OP_JZ:
    case OP_JE:
      // the address to jmp should follow the instruction
      if (core->fregr.zero == 1)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JNZ:
    case OP_JNE:
      // the address to jmp should follow the instruction
      if (core->fregr.zero == 0)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JNC:
      if (core->fregr.carry == 0)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JC:
      if (core->fregr.carry == 1)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JNO:
      if (core->fregr.overflow == 0)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JO:
      if (core->fregr.overflow == 1)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JNN:
      if (core->fregr.negative == 0)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JN:
      if (core->fregr.negative == 1)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JS:
    case OP_JNG:
      if (core->fregr.negative == 1)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JNS:
    case OP_JG:
      if (core->fregr.negative == 0)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JGE:
      if (core->fregr.negative == 0 || core->fregr.zero == 0)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_JSE:
      if (core->fregr.negative == 1 || core->fregr.zero == 0)
        core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
      break;
    case OP_CALL:
      call64(core, current);
      break;
    case OP_RET:
      ret64(core, current);
      break;
    case OP_RETZ:
    case OP_RETE:
      if (core->fregr.zero == 1)
        ret64(core, current);
      break;
    case OP_RETNZ:
    case OP_RETNE:
      if (core->fregr.zero == 0)
        ret64(core, current);
      break;
    case OP_RETNC:
      if (core->fregr.carry == 0)
        ret64(core, current);
      break;
    case OP_RETC:
      if (core->fregr.carry == 1)
        ret64(core, current);
      break;
    case OP_RETNO:
      if (core->fregr.overflow == 0)
        ret64(core, current);
      break;
    case OP_RETO:
      if (core->fregr.overflow == 1)
        ret64(core, current);
      break;
    case OP_RETNN:
      if (core->fregr.negative == 0)
        ret64(core, current);
      break;
    case OP_RETN:
      if (core->fregr.negative == 1)
        ret64(core, current);
      break;
    case OP_RETS:
    case OP_RETNG:
      if (core->fregr.negative == 1)
        ret64(core, current);
      break;
    case OP_RETNS:
    case OP_RETG:
      if (core->fregr.negative == 0)
        ret64(core, current);
      break;
    case OP_RETGE:
      if (core->fregr.negative == 0 || core->fregr.zero == 1)
        ret64(core, current);
      break;
    case OP_RETSE:
      if (core->fregr.negative == 1 || core->fregr.zero == 1)
        ret64(core, current);
      break;
    case OP_LOOP:
      if (core->regr[current.bytes.b1 & GPC_64_LAST_REGR] != 0)
        core->pc = current.whole_word & 0xFFFFFFFFFFFF;
      break;
    case OP_CALL_REG:
      call_reg64(core, current);
      break;
    case OP_JMP_REGR:
      core->pc = core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      break;
    case OP_INTR:
      core->req->type = PROGRAM_REQUEST;
      core->req->args[0] = current.half_words.w1;
      core->req->args[1] = core->regr[R1];
      core->req->args[2] = core->regr[R2];
      core->req->args[3] = core->regr[R3];
      core->req->args[4] = core->regr[R11];
      core->req->args[5] = core->regr[R12];
      core->regr[R0] = (merry_SEND_REQUEST(core->req) == RET_FAILURE)
                           ? REQUEST_FAILED
                           : core->req->args[0];
      core->regr[R1] = core->req->args[1];
      break;
    case OP_PUSH_IMM8:
      push_immb64(core, current);
      break;
    case OP_PUSH_IMM16:
      push_immw64(core, current);
      break;
    case OP_PUSH_IMM32:
      push_immd64(core, current);
      break;
    case OP_PUSH_IMM64:
      push_immq64(core, current);
      break;
    case OP_PUSH_REG:
      push_reg64(core, current);
      break;
    case OP_POP8:
      popb64(core, current);
      break;
    case OP_POP16:
      popw64(core, current);
      break;
    case OP_POP32:
      popd64(core, current);
      break;
    case OP_POP64:
      popq64(core, current);
      break;
    case OP_PUSHA:
      pusha64(core, current);
      break;
    case OP_POPA:
      popa64(core, current);
      break;
    case OP_PUSH_MEMB:
      push_memb64(core, current);
      break;
    case OP_PUSH_MEMW:
      push_memw64(core, current);
      break;
    case OP_PUSH_MEMD:
      push_memd64(core, current);
      break;
    case OP_PUSH_MEMQ:
      push_memq64(core, current);
      break;
    case OP_POP_MEMB:
      pop_memb64(core, current);
      break;
    case OP_POP_MEMW:
      pop_memw64(core, current);
      break;
    case OP_POP_MEMD:
      pop_memd64(core, current);
      break;
    case OP_POP_MEMQ:
      pop_memq64(core, current);
      break;
    case OP_LOADSB:
      loadsb64(core, current);
      break;
    case OP_LOADSW:
      loadsw64(core, current);
      break;
    case OP_LOADSD:
      loadsd64(core, current);
      break;
    case OP_LOADSQ:
      loadsq64(core, current);
      break;
    case OP_STORESB:
      storesb64(core, current);
      break;
    case OP_STORESW:
      storesw64(core, current);
      break;
    case OP_STORESD:
      storesd64(core, current);
      break;
    case OP_STORESQ:
      storesq64(core, current);
      break;
    case OP_AND_IMM:
      cmp_imm64(core, current);
      break;
    case OP_AND_REG:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] &=
          core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      break;
    case OP_OR_IMM:
      or_imm64(core, current);
      break;
    case OP_OR_REG:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] |=
          core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      break;
    case OP_XOR_IMM:
      xor_imm64(core, current);
      break;
    case OP_XOR_REG:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] ^=
          core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      break;
    case OP_NOT:
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR] =
          ~core->regr[current.bytes.b7];
      break;
    case OP_LSHIFT:
      core->regr[current.bytes.b1 & GPC_64_LAST_REGR] <<= current.bytes.b7 & 63;
      break;
    case OP_RSHIFT:
      core->regr[current.bytes.b1 & GPC_64_LAST_REGR] >>= current.bytes.b7 & 63;
      break;
    case OP_LSHIFT_REGR:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] <<=
          core->regr[current.bytes.b7 & GPC_64_LAST_REGR] & 63;
      break;
    case OP_RSHIFT_REGR:
      core->regr[current.bytes.b6 & GPC_64_LAST_REGR] >>=
          core->regr[current.bytes.b7 & GPC_64_LAST_REGR] & 63;
      break;
    case OP_CMP_IMM:
      cmp_imm64(core, current);
      break;
    case OP_CMP_REG:
      merry_compare_two_values(core->regr[current.bytes.b6 & GPC_64_LAST_REGR],
                               core->regr[current.bytes.b7 & GPC_64_LAST_REGR],
                               &core->fregr);
      break;
    case OP_CMP_IMM_MEMB:
      cmp_imm_memb64(core, current);
      break;
    case OP_CMP_IMM_MEMW:
      cmp_imm_memw64(core, current);
      break;
    case OP_CMP_IMM_MEMD:
      cmp_imm_memd64(core, current);
      break;
    case OP_CMP_IMM_MEMQ:
      cmp_imm_memq64(core, current);
      break;
    case OP_FCMP:
      merry_compare_f32(core->regr[current.bytes.b6 & GPC_64_LAST_REGR],
                        core->regr[current.bytes.b7 & GPC_64_LAST_REGR],
                        &core->ffregr);
      break;
    case OP_FCMP32:
      merry_compare_f32(core->regr[current.bytes.b6 & GPC_64_LAST_REGR],
                        core->regr[current.bytes.b7 & GPC_64_LAST_REGR],
                        &core->ffregr);
      break;
    case OP_CIN:
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR] = getchar();
      break;
    case OP_COUT:
      putchar((int)core->regr[current.bytes.b7 & GPC_64_LAST_REGR]);
      break;
    case OP_SIN_REG:
      current.whole_word = (core->regr[current.bytes.b7 & GPC_64_LAST_REGR]);
    case OP_SIN:
      sin64(core, current);
      break;
    case OP_SOUT_REG:
      current.bytes.b7 =
          (core->regr[current.bytes.b7 & GPC_64_LAST_REGR] & 0xFFFFFFFFFFFF);
    case OP_SOUT:
      sout64(core, current);
      break;
    case OP_IN:
      fscanf(stdin, "%hhi", &mem.bytes.b7);
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR] = mem.whole_word;
      break;
    case OP_OUT:
      mem.whole_word = core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      fprintf(stdout, "%hhi", mem.bytes.b7);
      break;
    case OP_INW:
      fscanf(stdin, "%hd", &mem.half_half_words.w3);
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR] = mem.whole_word;
      break;
    case OP_OUTW:
      mem.whole_word = core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      fprintf(stdout, "%hd", mem.half_half_words.w3);
      break;
    case OP_IND:
      fscanf(stdin, "%d", &mem.half_words.w1);
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR] = mem.whole_word;
      break;
    case OP_OUTD:
      mem.whole_word = core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      fprintf(stdout, "%d", mem.half_words.w1);
      break;
    case OP_INQ:
      fscanf(stdin, "%zi", &core->regr[current.bytes.b7 & GPC_64_LAST_REGR]);
      break;
    case OP_OUTQ:
      fprintf(stdout, "%zi", core->regr[current.bytes.b7 & GPC_64_LAST_REGR]);
      break;
    case OP_UIN:
      fscanf(stdin, "%hhu", &mem.bytes.b7);
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR] = mem.whole_word;
      break;
    case OP_UOUT:
      mem.whole_word = core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      fprintf(stdout, "%hhu", mem.bytes.b7);
      break;
    case OP_UINW:
      fscanf(stdin, "%hu", &mem.half_half_words.w3);
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR] = mem.whole_word;
      break;
    case OP_UOUTW:
      mem.whole_word = core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      fprintf(stdout, "%hu", mem.half_half_words.w3);
      break;
    case OP_UIND:
      fscanf(stdin, "%u", &mem.half_words.w1);
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR] = mem.whole_word;
      break;
    case OP_UOUTD:
      mem.whole_word = core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      fprintf(stdout, "%u", mem.half_words.w1);
      break;
    case OP_UINQ:
      fscanf(stdin, "%zu", &core->regr[current.bytes.b7 & GPC_64_LAST_REGR]);
      break;
    case OP_UOUTQ:
      fprintf(stdout, "%zu", core->regr[current.bytes.b7 & GPC_64_LAST_REGR]);
      break;
    case OP_INF:
      fscanf(stdin, "%lf", &dtoq.d_val);
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR] = dtoq.q_val;
      break;
    case OP_OUTF:
      dtoq.q_val = core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      fprintf(stdout, "%lf", dtoq.d_val);
      break;
    case OP_INF32:
      fscanf(stdin, "%f", &ftod.fl_val);
      core->regr[current.bytes.b7 & GPC_64_LAST_REGR] = ftod.d_val;
      break;
    case OP_OUTF32:
      ftod.d_val = core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      fprintf(stdout, "%f", ftod.fl_val);
      break;
    case OP_OUTR:
      for (msize_t i = 0; i <= REG_COUNT_GPC_64; i++)
        fprintf(stdout, "%zi\n", core->regr[i]);
      break;
    case OP_UOUTR:
      for (msize_t i = 0; i <= REG_COUNT_GPC_64; i++)
        fprintf(stdout, "%zu\n", core->regr[i]);
      break;
    case OP_LOADB:
      loadb64(core, current);
      break;
    case OP_LOADW:
      loadw64(core, current);
      break;
    case OP_LOADD:
      loadd64(core, current);
      break;
    case OP_LOADQ:
      loadq64(core, current);
      break;
    case OP_STOREB:
      storeb64(core, current);
      break;
    case OP_STOREW:
      storew64(core, current);
      break;
    case OP_STORED:
      stored64(core, current);
      break;
    case OP_STOREQ:
      storeq64(core, current);
      break;
    case OP_LOADB_REG:
      loadb_reg64(core, current);
      break;
    case OP_LOADW_REG:
      loadw_reg64(core, current);
      break;
    case OP_LOADD_REG:
      loadd_reg64(core, current);
      break;
    case OP_LOADQ_REG:
      loadq_reg64(core, current);
      break;
    case OP_STOREB_REG:
      storeb_reg64(core, current);
      break;
    case OP_STOREW_REG:
      storew_reg64(core, current);
      break;
    case OP_STORED_REG:
      stored_reg64(core, current);
      break;
    case OP_STOREQ_REG:
      storeq_reg64(core, current);
      break;
    case OP_ATOMIC_LOADB:
      atm_loadb64(core, current);
      break;
    case OP_ATOMIC_LOADW:
      atm_loadw64(core, current);
      break;
    case OP_ATOMIC_LOADD:
      atm_loadd64(core, current);
      break;
    case OP_ATOMIC_LOADQ:
      atm_loadq64(core, current);
      break;
    case OP_ATOMIC_STOREB:
      atm_storeb64(core, current);
      break;
    case OP_ATOMIC_STOREW:
      atm_storew64(core, current);
      break;
    case OP_ATOMIC_STORED:
      atm_stored64(core, current);
      break;
    case OP_ATOMIC_STOREQ:
      atm_storeq64(core, current);
      break;
    case OP_ATOMIC_LOADB_REG:
      atm_loadb_reg64(core, current);
      break;
    case OP_ATOMIC_LOADW_REG:
      atm_loadw_reg64(core, current);
      break;
    case OP_ATOMIC_LOADD_REG:
      atm_loadd_reg64(core, current);
      break;
    case OP_ATOMIC_LOADQ_REG:
      atm_loadq_reg64(core, current);
      break;
    case OP_ATOMIC_STOREB_REG:
      atm_storeb_reg64(core, current);
      break;
    case OP_ATOMIC_STOREW_REG:
      atm_storew_reg64(core, current);
      break;
    case OP_ATOMIC_STORED_REG:
      atm_stored_reg64(core, current);
      break;
    case OP_ATOMIC_STOREQ_REG:
      atm_storeq_reg64(core, current);
      break;
    case OP_LEA:
      core->regr[current.bytes.b4 & GPC_64_LAST_REGR] =
          core->regr[current.bytes.b5 & GPC_64_LAST_REGR] +
          core->regr[current.bytes.b6 & GPC_64_LAST_REGR] *
              core->regr[current.bytes.b7 & GPC_64_LAST_REGR];
      break;
    case OP_CFLAGS:
      core->fregr.carry = 0;
      core->fregr.negative = 0;
      core->fregr.zero = 0;
      core->fregr.overflow = 0;
      break;
    case OP_RESET:
      for (msize_t i = 0; i < REG_COUNT_GPC_64; i++)
        core->regr[i] = 0;
      break;
    case OP_CMPXCHG_REGR:
      cmpxchg_reg64(core, current);
      break;
    case OP_CMPXCHG:
      cmpxchg64(core, current);
      break;
    case OP_WHDLR:
      whdlr64(core, current);
      break;
    case 250:
    case 251:
    case 252:
    case 253:
    case 254:
      break;
    case 255: {
      switch (current.bytes.b1) {
      case OP_MOVFZ:
        if (core->ffregr.zf)
          mov_imm64(core, current);
        break;
      case OP_MOVFNZ:
        if (!core->ffregr.zf)
          mov_imm64(core, current);
        break;
      case OP_MOVFN:
        if (core->ffregr.sf)
          mov_imm64(core, current);
        break;
      case OP_MOVFNN:
        if (!core->ffregr.sf)
          mov_imm64(core, current);
        break;
      case OP_MOVFUF:
        if (core->ffregr.uof)
          mov_imm64(core, current);
        break;
      case OP_MOVFNUF:
        if (!core->ffregr.uof)
          mov_imm64(core, current);
        break;
      case OP_MOVFO:
        if (core->ffregr.of)
          mov_imm64(core, current);
        break;
      case OP_MOVFNO:
        if (!core->ffregr.of)
          mov_imm64(core, current);
        break;
      case OP_MOVFU:
        if (core->ffregr.uf)
          mov_imm64(core, current);
        break;
      case OP_MOVFNU:
        if (!core->ffregr.uf)
          mov_imm64(core, current);
        break;
      case OP_MOVFI:
        if (core->ffregr.inv)
          mov_imm64(core, current);
        break;
      case OP_MOVFNI:
        if (!core->ffregr.inv)
          mov_imm64(core, current);
        break;
      case OP_JFZ:
        if (core->ffregr.zf)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFNZ:
        if (!core->ffregr.zf)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFN:
        if (core->ffregr.sf)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFNN:
        if (!core->ffregr.sf)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFUF:
        if (core->ffregr.uof)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFNUF:
        if (!core->ffregr.uof)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFO:
        if (core->ffregr.of)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFNO:
        if (!core->ffregr.of)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFU:
        if (core->ffregr.uf)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFNU:
        if (!core->ffregr.uf)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFI:
        if (core->ffregr.inv)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_JFNI:
        if (!core->ffregr.inv)
          core->pc = (current.whole_word & 0xFFFFFFFFFFFF);
        break;
      case OP_RETFZ:
        if (core->ffregr.zf)
          ret64(core, current);
        break;
      case OP_RETFNZ:
        if (!core->ffregr.zf)
          ret64(core, current);
        break;
      case OP_RETFN:
        if (core->ffregr.sf)
          ret64(core, current);
        break;
      case OP_RETFNN:
        if (!core->ffregr.sf)
          ret64(core, current);
        break;
      case OP_RETFUF:
        if (core->ffregr.uof)
          ret64(core, current);
        break;
      case OP_RETFNUF:
        if (!core->ffregr.uof)
          ret64(core, current);
        break;
      case OP_RETFO:
        if (core->ffregr.of)
          ret64(core, current);
        break;
      case OP_RETFNO:
        if (!core->ffregr.of)
          ret64(core, current);
        break;
      case OP_RETFU:
        if (core->ffregr.uf)
          ret64(core, current);
        break;
      case OP_RETFNU:
        if (!core->ffregr.uf)
          ret64(core, current);
        break;
      case OP_RETFI:
        if (core->ffregr.inv)
          ret64(core, current);
        break;
      case OP_RETFNI:
        if (!core->ffregr.inv)
          ret64(core, current);
        break;
      }
      break;
    }
    }
    core->pc += 8;
  }
  return (_THRET_T_)(&core->regr[R0]);
}

EXEC64(add_imm) {
  register mbyte_t op1 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mqword_t op2;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &op2) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] += op2;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(add_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  core->regr[op1] += core->regr[op2];
  merry_update_flags_regr(&core->fregr);
}

EXEC64(sub_imm) {
  register mbyte_t op1 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mqword_t op2;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &op2) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] -= op2;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(sub_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  core->regr[op1] -= core->regr[op2];
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mul_imm) {
  register mbyte_t op1 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mqword_t op2;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &op2) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] *= op2;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mul_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  core->regr[op1] *= core->regr[op2];
  merry_update_flags_regr(&core->fregr);
}

EXEC64(div_imm) {
  register mbyte_t op1 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mqword_t op2;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &op2) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (op2 == 0) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] /= op2;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(div_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (core->regr[op2] == 0) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] /= core->regr[op2];
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mod_imm) {
  register mbyte_t op1 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mqword_t op2;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &op2) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (op2 == 0) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] %= op2;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mod_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (core->regr[op2] == 0) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] %= core->regr[op2];
  merry_update_flags_regr(&core->fregr);
}

EXEC64(iadd_imm) {
  register msqword_t op1 = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  msqword_t op2;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     (mqptr_t)&op2) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] = (mqword_t)(op1 + op2);
  merry_update_flags_regr(&core->fregr);
}

EXEC64(iadd_reg) {
  register msqword_t op1 = core->regr[inst.bytes.b6 & GPC_64_LAST_REGR];
  register msqword_t op2 = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  core->regr[inst.bytes.b6 & GPC_64_LAST_REGR] = (mqword_t)(op1 + op2);
  merry_update_flags_regr(&core->fregr);
}

EXEC64(isub_imm) {
  register msqword_t op1 = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  msqword_t op2;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     (mqptr_t)&op2) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] = (mqword_t)(op1 - op2);
  merry_update_flags_regr(&core->fregr);
}

EXEC64(isub_reg) {
  register msqword_t op1 = core->regr[inst.bytes.b6 & GPC_64_LAST_REGR];
  register msqword_t op2 = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  core->regr[inst.bytes.b6 & GPC_64_LAST_REGR] = (mqword_t)(op1 - op2);
  merry_update_flags_regr(&core->fregr);
}

EXEC64(imul_imm) {
  register msqword_t op1 = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  msqword_t op2;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     (mqptr_t)&op2) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] = (mqword_t)(op1 * op2);
  merry_update_flags_regr(&core->fregr);
}

EXEC64(imul_reg) {
  register msqword_t op1 = core->regr[inst.bytes.b6 & GPC_64_LAST_REGR];
  register msqword_t op2 = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  core->regr[inst.bytes.b6 & GPC_64_LAST_REGR] = (mqword_t)(op1 * op2);
  merry_update_flags_regr(&core->fregr);
}

EXEC64(idiv_imm) {
  register msqword_t op1 = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  msqword_t op2;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     (mqptr_t)&op2) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (op2 == 0) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] = (mqword_t)(op1 / op2);
  merry_update_flags_regr(&core->fregr);
}

EXEC64(idiv_reg) {
  register msqword_t op1 = core->regr[inst.bytes.b6 & GPC_64_LAST_REGR];
  register msqword_t op2 = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  if (op2 == 0) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b6 & GPC_64_LAST_REGR] = (mqword_t)(op1 / op2);
  merry_update_flags_regr(&core->fregr);
}

EXEC64(imod_imm) {
  register msqword_t op1 = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  msqword_t op2;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     (mqptr_t)&op2) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (op2 == 0) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] = (mqword_t)(op1 % op2);
  merry_update_flags_regr(&core->fregr);
}

EXEC64(imod_reg) {
  register msqword_t op1 = core->regr[inst.bytes.b6 & GPC_64_LAST_REGR];
  register msqword_t op2 = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  if (op2 == 0) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b6 & GPC_64_LAST_REGR] = (mqword_t)(op1 % op2);
  merry_update_flags_regr(&core->fregr);
}

EXEC64(fadd) {
  register mqword_t op1 = (inst.bytes.b6) & GPC_64_LAST_REGR;
  register mqword_t op2 = (inst.bytes.b7) & GPC_64_LAST_REGR;
  register MerryDoubleToQword a;
  register MerryDoubleToQword b;
  a.q_val = core->regr[op1];
  b.q_val = core->regr[op2];
  merry_compare_f64(a.d_val, b.d_val, &core->ffregr);
  a.d_val += b.d_val;
  core->regr[op1] = a.q_val;
}

EXEC64(fsub) {
  register mqword_t op1 = (inst.bytes.b6) & GPC_64_LAST_REGR;
  register mqword_t op2 = (inst.bytes.b7) & GPC_64_LAST_REGR;
  register MerryDoubleToQword a;
  register MerryDoubleToQword b;
  a.q_val = core->regr[op1];
  b.q_val = core->regr[op2];
  merry_compare_f64(a.d_val, b.d_val, &core->ffregr);
  a.d_val -= b.d_val;
  core->regr[op1] = a.q_val;
}

EXEC64(fmul) {
  register mqword_t op1 = (inst.bytes.b6) & GPC_64_LAST_REGR;
  register mqword_t op2 = (inst.bytes.b7) & GPC_64_LAST_REGR;
  register MerryDoubleToQword a;
  register MerryDoubleToQword b;
  a.q_val = core->regr[op1];
  b.q_val = core->regr[op2];
  merry_compare_f64(a.d_val, b.d_val, &core->ffregr);
  a.d_val *= b.d_val;
  core->regr[op1] = a.q_val;
}

EXEC64(fdiv) {
  register mqword_t op1 = (inst.bytes.b6) & GPC_64_LAST_REGR;
  register mqword_t op2 = (inst.bytes.b7) & GPC_64_LAST_REGR;
  register MerryDoubleToQword a;
  register MerryDoubleToQword b;
  a.q_val = core->regr[op1];
  b.q_val = core->regr[op2];
  if (b.d_val == 0.0) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  merry_compare_f64(a.d_val, b.d_val, &core->ffregr);
  a.d_val /= b.d_val;
  core->regr[op1] = a.q_val;
}

EXEC64(fadd32) {
  register mqword_t op1 = (inst.bytes.b6) & GPC_64_LAST_REGR;
  register mqword_t op2 = (inst.bytes.b7) & GPC_64_LAST_REGR;
  register MerryFloatToDword a;
  register MerryFloatToDword b;
  a.d_val = core->regr[op1];
  b.d_val = core->regr[op2];
  merry_compare_f32(a.fl_val, b.fl_val, &core->ffregr);
  a.fl_val += b.fl_val;
  core->regr[op1] = a.d_val;
}

EXEC64(fsub32) {
  register mqword_t op1 = (inst.bytes.b6) & GPC_64_LAST_REGR;
  register mqword_t op2 = (inst.bytes.b7) & GPC_64_LAST_REGR;
  register MerryFloatToDword a;
  register MerryFloatToDword b;
  a.d_val = core->regr[op1];
  b.d_val = core->regr[op2];
  merry_compare_f32(a.fl_val, b.fl_val, &core->ffregr);
  a.fl_val -= b.fl_val;
  core->regr[op1] = a.d_val;
}

EXEC64(fmul32) {
  register mqword_t op1 = (inst.bytes.b6) & GPC_64_LAST_REGR;
  register mqword_t op2 = (inst.bytes.b7) & GPC_64_LAST_REGR;
  register MerryFloatToDword a;
  register MerryFloatToDword b;
  a.d_val = core->regr[op1];
  b.d_val = core->regr[op2];
  merry_compare_f32(a.fl_val, b.fl_val, &core->ffregr);
  a.fl_val *= b.fl_val;
  core->regr[op1] = a.d_val;
}

EXEC64(fdiv32) {
  register mqword_t op1 = (inst.bytes.b6) & GPC_64_LAST_REGR;
  register mqword_t op2 = (inst.bytes.b7) & GPC_64_LAST_REGR;
  register MerryFloatToDword a;
  register MerryFloatToDword b;
  a.d_val = core->regr[op1];
  b.d_val = core->regr[op2];
  if (b.d_val == 0.0) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  merry_compare_f32(a.fl_val, b.fl_val, &core->ffregr);
  a.fl_val /= b.fl_val;
  core->regr[op1] = a.d_val;
}

EXEC64(add_memb) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mbyte_t temp = 0;
  if (merry_core_mem_access_GET_DATA_byte(core->req, &core->base->state,
                                          core->base->ram, addr,
                                          &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] += temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(add_memw) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_word(core->req, &core->base->state,
                                          core->base->ram, addr,
                                          &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] += temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(add_memd) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mdword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] += temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(add_memq) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_qword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] += temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(sub_memb) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mbyte_t temp = 0;
  if (merry_core_mem_access_GET_DATA_byte(core->req, &core->base->state,
                                          core->base->ram, addr,
                                          &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] -= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(sub_memw) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_word(core->req, &core->base->state,
                                          core->base->ram, addr,
                                          &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] -= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(sub_memd) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mdword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] -= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(sub_memq) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_qword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] -= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mul_memb) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mbyte_t temp = 0;
  if (merry_core_mem_access_GET_DATA_byte(core->req, &core->base->state,
                                          core->base->ram, addr,
                                          &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] *= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mul_memw) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_word(core->req, &core->base->state,
                                          core->base->ram, addr,
                                          &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] *= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mul_memd) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mdword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] *= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mul_memq) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_qword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] *= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(div_memb) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mbyte_t temp = 0;
  if (merry_core_mem_access_GET_DATA_byte(core->req, &core->base->state,
                                          core->base->ram, addr,
                                          &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (temp == 0) {
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] /= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(div_memw) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_word(core->req, &core->base->state,
                                          core->base->ram, addr,
                                          &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (temp == 0) {
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] /= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(div_memd) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mdword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (temp == 0) {
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] /= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(div_memq) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_qword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (temp == 0) {
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] /= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mod_memb) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mbyte_t temp = 0;
  if (merry_core_mem_access_GET_DATA_byte(core->req, &core->base->state,
                                          core->base->ram, addr,
                                          &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (temp == 0) {
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] %= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mod_memw) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_word(core->req, &core->base->state,
                                          core->base->ram, addr,
                                          &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (temp == 0) {
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] %= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mod_memd) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mdword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (temp == 0) {
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] %= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(mod_memq) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register maddress_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_core_mem_access_GET_DATA_qword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (temp == 0) {
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] %= temp;
  merry_update_flags_regr(&core->fregr);
}

EXEC64(fadd64_mem) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register mqword_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  MerryDoubleToQword temp;
  if (merry_core_mem_access_GET_DATA_qword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp.q_val) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  MerryDoubleToQword rval;
  rval.q_val = core->regr[op1];

  merry_compare_f64(rval.d_val, temp.d_val, &core->ffregr);
  temp.d_val += rval.d_val;
  core->regr[op1] = temp.q_val;
}

EXEC64(fsub64_mem) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register mqword_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  MerryDoubleToQword temp;
  if (merry_core_mem_access_GET_DATA_qword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp.q_val) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  MerryDoubleToQword rval;
  rval.q_val = core->regr[op1];

  merry_compare_f64(rval.d_val, temp.d_val, &core->ffregr);
  temp.d_val -= rval.d_val;
  core->regr[op1] = temp.q_val;
}

EXEC64(fmul64_mem) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register mqword_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  MerryDoubleToQword temp;
  if (merry_core_mem_access_GET_DATA_qword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp.q_val) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  MerryDoubleToQword rval;
  rval.q_val = core->regr[op1];

  merry_compare_f64(rval.d_val, temp.d_val, &core->ffregr);
  temp.d_val *= rval.d_val;
  core->regr[op1] = temp.q_val;
}

EXEC64(fdiv64_mem) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register mqword_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  MerryDoubleToQword temp;
  if (merry_core_mem_access_GET_DATA_qword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp.q_val) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (temp.d_val == 0.0) {
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  MerryDoubleToQword rval;
  rval.q_val = core->regr[op1];

  merry_compare_f64(rval.d_val, temp.d_val, &core->ffregr);
  temp.d_val /= rval.d_val;
  core->regr[op1] = temp.q_val;
}

EXEC64(fadd32_mem) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register mqword_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  MerryFloatToDword temp;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp.d_val) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  MerryFloatToDword rval;
  rval.d_val = core->regr[op1];

  merry_compare_f32(rval.fl_val, temp.fl_val, &core->ffregr);
  temp.fl_val += rval.fl_val;
  core->regr[op1] = temp.d_val;
  core->regr[op1] = temp.d_val;
}

EXEC64(fsub32_mem) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register mqword_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  MerryFloatToDword temp;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp.d_val) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  MerryFloatToDword rval;
  rval.d_val = core->regr[op1];

  merry_compare_f32(rval.fl_val, temp.fl_val, &core->ffregr);
  temp.fl_val -= rval.fl_val;
  core->regr[op1] = temp.d_val;
  core->regr[op1] = temp.d_val;
}

EXEC64(fmul32_mem) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register mqword_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  MerryFloatToDword temp;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp.d_val) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  MerryFloatToDword rval;
  rval.d_val = core->regr[op1];

  merry_compare_f32(rval.fl_val, temp.fl_val, &core->ffregr);
  temp.fl_val *= rval.fl_val;
  core->regr[op1] = temp.d_val;
  core->regr[op1] = temp.d_val;
}

EXEC64(fdiv32_mem) {
  register mqword_t op1 = (inst.bytes.b1) & GPC_64_LAST_REGR;
  register mqword_t addr = (inst.whole_word & 0xFFFFFFFFFFFF);
  MerryFloatToDword temp;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram, addr,
                                           &temp.d_val) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (temp.d_val == 0.0) {
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_, _DIV_BY_ZERO_);
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  MerryFloatToDword rval;
  rval.d_val = core->regr[op1];

  merry_compare_f32(rval.fl_val, temp.fl_val, &core->ffregr);
  temp.fl_val /= rval.fl_val;
  core->regr[op1] = temp.d_val;
  core->regr[op1] = temp.d_val;
}

EXEC64(mov_imm) {
  register mbyte_t op1 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mqword_t val;

  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &val) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }

  core->regr[op1] = val;
}

EXEC64(movf32) {
  register mbyte_t op1 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mqword_t val;

  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &val) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }

  core->regr[op1] = val & 0xFFFFFFFF;
}

EXEC64(movesx_imm8) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  register mqword_t op2 = inst.bytes.b7;
  sign_extend8(op2);
  core->regr[op1] = op2;
}

EXEC64(movesx_imm16) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  register mqword_t op2 = inst.half_half_words.w3;
  sign_extend16(op2);
  core->regr[op1] = op2;
}

EXEC64(movesx_imm32) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  register mqword_t op2 = inst.half_words.w1;
  sign_extend32(op2);
  core->regr[op1] = op2;
}

EXEC64(movesx_reg8) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  sign_extend8(core->regr[op2]);
  core->regr[op1] = op2;
}

EXEC64(movesx_reg16) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  sign_extend16(core->regr[op2]);
  core->regr[op1] = op2;
}

EXEC64(movesx_reg32) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  sign_extend32(core->regr[op2]);
  core->regr[op1] = op2;
}

EXEC64(excg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  register mqword_t temp = core->regr[op1];
  core->regr[op1] = core->regr[op2];
  core->regr[op2] = temp;
}

EXEC64(excg8) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  register mqword_t r1 = core->regr[op1];
  core->regr[op1] |= core->regr[op2] & 0x00000000000000FF;
  core->regr[op2] |= r1 & 0x00000000000000FF;
}

EXEC64(excg16) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  register mqword_t r1 = core->regr[op1];
  core->regr[op1] |= core->regr[op2] & 0x000000000000FFFF;
  core->regr[op2] |= r1 & 0x000000000000FFFF;
}

EXEC64(excg32) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  register mqword_t r1 = core->regr[op1];
  core->regr[op1] |= core->regr[op2] & 0x00000000FFFFFFFF;
  core->regr[op2] |= r1 & 0x00000000FFFFFFFF;
}

EXEC64(call) {
  mqword_t addr = inst.whole_word & 0xFFFFFFFFFFFF;
  if (merry_stack_push(core->tbs, (mptr_t)core->pc) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_stack_push(core->cstack, (mptr_t)addr) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->pc = addr - 8; // the address to the first instruction of the procedure
}

EXEC64(ret) {
  MerryPtrToQword a;

  // restore PC
  if ((a.ptr = merry_stack_pop(core->tbs)) == RET_NULL) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_,
                       _INVALID_PROCEDURE_RETURN_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    return;
  }

  // if the above failed then this will also fail and the failure
  // of above won't reach this
  if ((merry_stack_pop(core->cstack)) == RET_NULL) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }

  core->pc = a.qword;
}

EXEC64(call_reg) {
  MerryPtrToQword addr;
  addr.qword = core->regr[inst.bytes.b7 & GPC_64_LAST_REGR];
  if (merry_stack_push(core->tbs, (mqptr_t)core->pc) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_stack_push(core->cstack, addr.ptr) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->pc = addr.qword - 8;
}

EXEC64(push_immb) {
  MerryHostMemLayout imm;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &imm.whole_word) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_RAM_write_byte(core->stack, core->regr[SP], imm.bytes.b7,
                           &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_OVERFLOW_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP]++;
}

EXEC64(push_immw) {
  MerryHostMemLayout imm;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &imm.whole_word) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_RAM_write_word(core->stack, core->regr[SP], imm.half_half_words.w3,
                           &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_OVERFLOW_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] += 2;
}

EXEC64(push_immd) {
  MerryHostMemLayout imm;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &imm.whole_word) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_RAM_write_dword(core->stack, core->regr[SP], imm.half_words.w1,
                            &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_OVERFLOW_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] += 4;
}

EXEC64(push_immq) {
  MerryHostMemLayout imm;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &imm.whole_word) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_RAM_write_qword(core->stack, core->regr[SP], imm.whole_word,
                            &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_OVERFLOW_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] += 8;
}

EXEC64(push_reg) {
  if (merry_RAM_write_qword(core->stack, core->regr[SP],
                            core->regr[inst.bytes.b7 & GPC_64_LAST_REGR],
                            &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_OVERFLOW_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] += 8;
}

EXEC64(popb) {
  if (core->regr[SP] == 0) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_UNDERFLOW_);
  }
  mbyte_t tmp = 0;
  if (merry_RAM_read_byte(core->stack, core->regr[SP], &tmp,
                          &core->base->state) == RET_FAILURE) {
    // This will not fail at all
    // i.e this section should never be executed
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] = tmp;
  core->regr[SP] -= 1;
}

EXEC64(popw) {
  if (core->regr[SP] == 0) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_UNDERFLOW_);
  }
  mword_t tmp = 0;
  if (merry_RAM_read_word(core->stack, core->regr[SP], &tmp,
                          &core->base->state) == RET_FAILURE) {
    // This will not fail at all
    // i.e this section should never be executed
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] = tmp;
  core->regr[SP] -= 2;
}

EXEC64(popd) {
  if (core->regr[SP] == 0) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_UNDERFLOW_);
  }
  mdword_t tmp = 0;
  if (merry_RAM_read_dword(core->stack, core->regr[SP], &tmp,
                           &core->base->state) == RET_FAILURE) {
    // This will not fail at all
    // i.e this section should never be executed
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] = tmp;
  core->regr[SP] -= 4;
}

EXEC64(popq) {
  if (core->regr[SP] == 0) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_UNDERFLOW_);
  }
  if (merry_RAM_read_qword(core->stack, core->regr[SP],
                           &core->regr[inst.bytes.b7 & GPC_64_LAST_REGR],
                           &core->base->state) == RET_FAILURE) {
    // This will not fail at all
    // i.e this section should never be executed
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] -= 8;
}

EXEC64(pusha) {
  for (msize_t i = 0; i < REG_COUNT_GPC_64; i++) {
    if (merry_RAM_write_qword(core->stack, core->regr[SP], core->regr[i],
                              &core->base->state) == RET_FAILURE) {
      merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_STACK_OVERFLOW_);
      merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
      core->req->type = PROBLEM_ENCOUNTERED;
      merry_SEND_REQUEST(core->req);
      core->base->stop = mtrue;
      core->base->terminate = mtrue;
      return;
    }
    core->regr[SP] += 8;
  }
}

EXEC64(popa) {
  if (core->regr[SP] < (REG_COUNT_GPC_64 * 8)) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_UNDERFLOW_);
  }
  for (msize_t i = REG_COUNT_GPC_64 - 1; i >= 0; i++) {
    if (merry_RAM_read_qword(core->stack, core->regr[SP], &core->regr[i],
                             &core->base->state) == RET_FAILURE) {
      merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_STACK_OVERFLOW_);
      merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
      core->req->type = PROBLEM_ENCOUNTERED;
      merry_SEND_REQUEST(core->req);
      core->base->stop = mtrue;
      core->base->terminate = mtrue;
      return;
    }
    core->regr[SP] -= 8;
  }
}

EXEC64(push_memb) {
  MerryHostMemLayout imm;
  if (merry_core_mem_access_GET_DATA_byte(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm.bytes.b7) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_RAM_write_byte(core->stack, core->regr[SP], imm.bytes.b7,
                           &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_OVERFLOW_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] += 1;
}

EXEC64(push_memw) {
  MerryHostMemLayout imm;
  if (merry_core_mem_access_GET_DATA_word(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF,
          &imm.half_half_words.w3) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_RAM_write_word(core->stack, core->regr[SP], imm.half_half_words.w3,
                           &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_OVERFLOW_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] += 2;
}

EXEC64(push_memd) {
  MerryHostMemLayout imm;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram,
                                           inst.whole_word & 0xFFFFFFFFFFFF,
                                           &imm.half_words.w1) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_RAM_write_dword(core->stack, core->regr[SP], imm.half_words.w1,
                            &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_OVERFLOW_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] += 4;
}

EXEC64(push_memq) {
  MerryHostMemLayout imm;
  if (merry_core_mem_access_GET_DATA_qword(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm.whole_word) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_RAM_write_qword(core->stack, core->regr[SP], imm.whole_word,
                            &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_OVERFLOW_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] += 8;
}

EXEC64(pop_memb) {
  if (core->regr[SP] == 0) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_UNDERFLOW_);
  }
  mbyte_t tmp;
  if (merry_RAM_read_byte(core->stack, core->regr[SP], &tmp,
                          &core->base->state) == RET_FAILURE) {
    // This will not fail at all
    // i.e this section should never be executed
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_core_mem_access_WRITE_DATA_byte(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, tmp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] -= 1;
}

EXEC64(pop_memw) {
  if (core->regr[SP] == 0) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_UNDERFLOW_);
  }
  mword_t tmp;
  if (merry_RAM_read_word(core->stack, core->regr[SP], &tmp,
                          &core->base->state) == RET_FAILURE) {
    // This will not fail at all
    // i.e this section should never be executed
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_core_mem_access_WRITE_DATA_word(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, tmp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] -= 2;
}

EXEC64(pop_memd) {
  if (core->regr[SP] == 0) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_UNDERFLOW_);
  }
  mdword_t tmp;
  if (merry_RAM_read_dword(core->stack, core->regr[SP], &tmp,
                           &core->base->state) == RET_FAILURE) {
    // This will not fail at all
    // i.e this section should never be executed
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_core_mem_access_WRITE_DATA_dword(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, tmp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] -= 4;
}

EXEC64(pop_memq) {
  if (core->regr[SP] == 0) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_STACK_UNDERFLOW_);
  }
  mqword_t tmp;
  if (merry_RAM_read_qword(core->stack, core->regr[SP], &tmp,
                           &core->base->state) == RET_FAILURE) {
    // This will not fail at all
    // i.e this section should never be executed
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  if (merry_core_mem_access_WRITE_DATA_qword(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, tmp) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[SP] -= 8;
}

EXEC64(loadsb) {
  register mqword_t off = inst.half_half_words.w1;
  sign_extend16(off);
  mbyte_t tmp;
  if (merry_RAM_read_byte(core->stack, core->regr[BP] + off, &tmp,
                          &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_,
                       _INVALID_STACK_ACCESS_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b1 & GPC_64_LAST_REGR] = tmp;
}

EXEC64(loadsw) {
  register mqword_t off = inst.half_half_words.w1;
  sign_extend16(off);
  mword_t tmp;
  if (merry_RAM_read_word(core->stack, core->regr[BP] + off, &tmp,
                          &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_,
                       _INVALID_STACK_ACCESS_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b1 & GPC_64_LAST_REGR] = tmp;
}

EXEC64(loadsd) {
  register mqword_t off = inst.half_half_words.w1;
  sign_extend16(off);
  mdword_t tmp;
  if (merry_RAM_read_dword(core->stack, core->regr[BP] + off, &tmp,
                           &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_,
                       _INVALID_STACK_ACCESS_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b1 & GPC_64_LAST_REGR] = tmp;
}

EXEC64(loadsq) {
  register mqword_t off = inst.half_half_words.w1;
  sign_extend16(off);
  mqword_t tmp;
  if (merry_RAM_read_qword(core->stack, core->regr[BP] + off, &tmp,
                           &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_,
                       _INVALID_STACK_ACCESS_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b1 & GPC_64_LAST_REGR] = tmp;
}

EXEC64(storesb) {
  register mqword_t off = inst.half_half_words.w1;
  sign_extend16(off);
  if (merry_RAM_write_byte(core->stack, core->regr[BP] + off,
                           core->regr[inst.bytes.b1 & GPC_64_LAST_REGR] & 0xFF,
                           &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_,
                       _INVALID_STACK_ACCESS_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(storesw) {
  register mqword_t off = inst.half_half_words.w1;
  sign_extend16(off);
  if (merry_RAM_write_word(core->stack, core->regr[BP] + off,
                           core->regr[inst.bytes.b1 & GPC_64_LAST_REGR] &
                               0xFFFF,
                           &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_,
                       _INVALID_STACK_ACCESS_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(storesd) {
  register mqword_t off = inst.half_half_words.w1;
  sign_extend16(off);
  if (merry_RAM_write_dword(core->stack, core->regr[BP] + off,
                            core->regr[inst.bytes.b1 & GPC_64_LAST_REGR] &
                                0xFFFFFFFF,
                            &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_,
                       _INVALID_STACK_ACCESS_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(storesq) {
  register mqword_t off = inst.half_half_words.w1;
  sign_extend16(off);
  if (merry_RAM_write_qword(core->stack, core->regr[BP] + off,
                            core->regr[inst.bytes.b1 & GPC_64_LAST_REGR],
                            &core->base->state) == RET_FAILURE) {
    merry_assign_state(core->base->state, _MERRY_PROGRAM_ERROR_,
                       _INVALID_STACK_ACCESS_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(and_imm) {
  mqword_t imm;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] &= imm;
}

EXEC64(or_imm) {
  mqword_t imm;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] |= imm;
}

EXEC64(xor_imm) {
  mqword_t imm;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[inst.bytes.b7 & GPC_64_LAST_REGR] ^= imm;
}

EXEC64(cmp_imm) {
  mqword_t imm;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->ram, core->pc,
                                     &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  merry_compare_two_values(imm, core->regr[inst.bytes.b7 & GPC_64_LAST_REGR],
                           &core->fregr);
}

EXEC64(cmp_imm_memb) {
  MerryHostMemLayout imm;
  if (merry_core_mem_access_GET_DATA_byte(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm.bytes.b7) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  merry_compare_two_values(imm.whole_word,
                           core->regr[inst.bytes.b1 & GPC_64_LAST_REGR],
                           &core->fregr);
}

EXEC64(cmp_imm_memw) {
  MerryHostMemLayout imm;
  if (merry_core_mem_access_GET_DATA_word(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF,
          &imm.half_half_words.w3) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  merry_compare_two_values(imm.whole_word,
                           core->regr[inst.bytes.b1 & GPC_64_LAST_REGR],
                           &core->fregr);
}

EXEC64(cmp_imm_memd) {
  MerryHostMemLayout imm;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram,
                                           inst.whole_word & 0xFFFFFFFFFFFF,
                                           &imm.half_words.w1) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  merry_compare_two_values(imm.whole_word,
                           core->regr[inst.bytes.b1 & GPC_64_LAST_REGR],
                           &core->fregr);
}

EXEC64(cmp_imm_memq) {
  MerryHostMemLayout imm;
  if (merry_core_mem_access_GET_DATA_qword(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm.whole_word) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  merry_compare_two_values(imm.whole_word,
                           core->regr[inst.bytes.b1 & GPC_64_LAST_REGR],
                           &core->fregr);
}

EXEC64(sin) {
  register mqword_t len = core->regr[R0];
  register mqword_t addr = inst.whole_word & 0xFFFFFFFFFFFF;
  mbptr_t temp = (mbptr_t)malloc(len);
  if (temp == NULL) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    merry_provide_context(core->base->state, _MERRY_CORE_EXECUTING_);
    core->req->type = PROBLEM_ENCOUNTERED;
    merry_SEND_REQUEST(core->req);
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  for (msize_t i = 0; i < len; i++) {
    temp[i] = getchar();
  }
  if (merry_core_base_mem_access_bulk_write(core->req, &core->base->state,
                                            core->base->ram, addr, temp,
                                            len) == RET_FAILURE)
    core->base->stop = mtrue;
  core->base->terminate = mtrue;
  free(temp);
}

EXEC64(sout) {
  register mqword_t len = core->regr[R0];
  register mqword_t addr = inst.whole_word & 0xFFFFFFFFFFFF;
  mbptr_t temp;
  if (merry_core_base_mem_access_bulk_read(core->req, &core->base->state,
                                           core->base->ram, addr, &temp,
                                           len) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
  } else {
    printf("%s", temp);
    free(temp);
  }
}

EXEC64(loadb) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  mbyte_t imm;
  if (merry_core_mem_access_GET_DATA_byte(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(storeb) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_byte(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF,
          core->regr[op1] & 0xFF) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(loadw) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  mword_t imm;
  if (merry_core_mem_access_GET_DATA_word(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(storew) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_word(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF,
          core->regr[op1] & 0xFFFF) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(loadd) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  mdword_t imm;
  if (merry_core_mem_access_GET_DATA_dword(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(stored) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_dword(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF,
          core->regr[op1] & 0xFFFFFFFF) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(loadq) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  mqword_t imm;
  if (merry_core_mem_access_GET_DATA_qword(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(storeq) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_qword(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, core->regr[op1]) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(loadb_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mbyte_t imm;
  if (merry_core_mem_access_GET_DATA_byte(core->req, &core->base->state,
                                          core->base->ram, core->regr[op2],
                                          &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(storeb_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_byte(
          core->req, &core->base->state, core->base->ram, core->regr[op2],
          core->regr[op1] & 0xFF) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(loadw_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mword_t imm;
  if (merry_core_mem_access_GET_DATA_word(core->req, &core->base->state,
                                          core->base->ram, core->regr[op2],
                                          &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(storew_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_word(
          core->req, &core->base->state, core->base->ram, core->regr[op2],
          core->regr[op1] & 0xFFFF) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(loadd_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mdword_t imm;
  if (merry_core_mem_access_GET_DATA_dword(core->req, &core->base->state,
                                           core->base->ram, core->regr[op2],
                                           &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(stored_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_dword(
          core->req, &core->base->state, core->base->ram, core->regr[op2],
          core->regr[op1] & 0xFFFFFFFF) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(loadq_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_GET_DATA_qword(core->req, &core->base->state,
                                           core->base->ram, core->regr[op2],
                                           &core->regr[op1]) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(storeq_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_qword(core->req, &core->base->state,
                                             core->base->ram, core->regr[op2],
                                             core->regr[op1]) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(atm_loadb) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  mbyte_t imm;
  if (merry_core_mem_access_GET_DATA_byte_atm(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(atm_loadw) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  mword_t imm;
  if (merry_core_mem_access_GET_DATA_word_atm(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(atm_loadd) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  mdword_t imm;
  if (merry_core_mem_access_GET_DATA_dword_atm(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(atm_loadq) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  mqword_t imm;
  if (merry_core_mem_access_GET_DATA_qword_atm(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(atm_storeb) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_byte_atm(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, core->regr[op1]) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(atm_storew) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_word_atm(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, core->regr[op1]) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(atm_stored) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_dword_atm(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, core->regr[op1]) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(atm_storeq) {
  register mbyte_t op1 = inst.bytes.b1 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_qword_atm(
          core->req, &core->base->state, core->base->ram,
          inst.whole_word & 0xFFFFFFFFFFFF, core->regr[op1]) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(atm_loadb_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mbyte_t imm;
  if (merry_core_mem_access_GET_DATA_byte_atm(core->req, &core->base->state,
                                              core->base->ram, core->regr[op2],
                                              &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(atm_loadw_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mword_t imm;
  if (merry_core_mem_access_GET_DATA_word_atm(core->req, &core->base->state,
                                              core->base->ram, core->regr[op2],
                                              &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(atm_loadd_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  mdword_t imm;
  if (merry_core_mem_access_GET_DATA_dword_atm(core->req, &core->base->state,
                                               core->base->ram, core->regr[op2],
                                               &imm) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  core->regr[op1] = imm;
}

EXEC64(atm_loadq_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_GET_DATA_qword_atm(
          core->req, &core->base->state, core->base->ram, core->regr[op2],
          &core->regr[op1]) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(atm_storeb_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_byte_atm(
          core->req, &core->base->state, core->base->ram, core->regr[op1],
          core->regr[op2] & 0xFF) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(atm_storew_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_word_atm(
          core->req, &core->base->state, core->base->ram, core->regr[op1],
          core->regr[op2] & 0xFFFF) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(atm_stored_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_dword_atm(
          core->req, &core->base->state, core->base->ram, core->regr[op1],
          core->regr[op2] & 0xFFFFFFFF) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(atm_storeq_reg) {
  register mbyte_t op1 = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t op2 = inst.bytes.b7 & GPC_64_LAST_REGR;
  if (merry_core_mem_access_WRITE_DATA_qword_atm(
          core->req, &core->base->state, core->base->ram, core->regr[op1],
          core->regr[op2]) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
}

EXEC64(cmpxchg) {
  register mbyte_t desired = inst.bytes.b6;
  register mbyte_t expected = inst.bytes.b7;
  mqword_t address;
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(core->req, &core->base->state,
                                     core->base->iram, core->pc,
                                     &address) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->terminate = mtrue;
    return;
  }
  MerryFlagsRegr reg = core->fregr;
  mret_t ret = merry_RAM_cmpxchg(core->base->ram, address, desired, expected,
                                 &core->base->state);
  merry_update_flags_regr(&core->fregr);
  if (ret == RET_FAILURE) {
    core->req->type = TRY_LOADING_NEW_PAGE_DATA;
    core->req->args[0] = address / _MERRY_PAGE_LEN_;
    if (merry_SEND_REQUEST(core->req) == RET_FAILURE ||
        core->req->args[0] == 1) {
      core->base->stop = mtrue;
      core->base->terminate = mtrue;
      core->fregr = reg;
      return;
    }
    ret = merry_RAM_cmpxchg(core->base->ram, address, desired, expected,
                            &core->base->state); // this shouldn't fail now
    merry_update_flags_regr(&core->fregr);
  }
}

EXEC64(cmpxchg_reg) {
  register mbyte_t desired = inst.bytes.b6 & GPC_64_LAST_REGR;
  register mbyte_t expected = inst.bytes.b7 & GPC_64_LAST_REGR;
  register mqword_t address = core->regr[inst.bytes.b5 & GPC_64_LAST_REGR];
  MerryFlagsRegr reg = core->fregr;
  mret_t ret = merry_RAM_cmpxchg(core->base->ram, address, desired, expected,
                                 &core->base->state);
  merry_update_flags_regr(&core->fregr);
  if (ret == RET_FAILURE) {
    core->req->type = TRY_LOADING_NEW_PAGE_DATA;
    core->req->args[0] = address / _MERRY_PAGE_LEN_;
    if (merry_SEND_REQUEST(core->req) == RET_FAILURE ||
        core->req->args[0] == 1) {
      core->base->stop = mtrue;
      core->fregr = reg;
      return;
    }
    ret = merry_RAM_cmpxchg(core->base->ram, address, desired, expected,
                            &core->base->state); // this shouldn't fail now
    merry_update_flags_regr(&core->fregr);
  }
}

EXEC64(whdlr) {
  core->pc += 8;
  if (merry_core_mem_access_GET_INST(
          core->req, &core->base->state, core->base->iram, core->pc,
          &core->base->wild_request_hdlr) == RET_FAILURE) {
    core->base->stop = mtrue;
    core->base->wild_request_hdlr_set = mfalse;
    return;
  }
  core->base->wild_request_hdlr_set = mtrue;
}

msize_t merry_64_bit_core_find_free_state(Merry64BitCore *core) {
  for (msize_t i = 0; i < merry_dynamic_list_size(core->base->execution_states);
       i++) {
    Merry64BitCoreState *st = (Merry64BitCoreState *)merry_dynamic_list_at(
        core->base->execution_states, i);
    if (st->state_valid) {
      return i;
    }
  }
  return (mqword_t)(-1);
}

mret_t merry_64_bit_core_save_state(void *cptr) {
  merry_check_ptr(cptr);
  Merry64BitCore *core = (Merry64BitCore *)cptr;

  Merry64BitCoreState st;
  st.state_valid = mfalse; // occupied state
  st.ffregr = core->ffregr;
  st.fregr = core->fregr;
  st.pc = core->pc;
  for (msize_t i = 0; i < REG_COUNT_GPC_64; i++) {
    st.regr[i] = core->regr[i];
  }

  // See if we have any saved states
  msize_t id;
  if ((id = merry_64_bit_core_find_free_state(core)) == (mqword_t)(-1)) {
    // Need a new state
    if (merry_dynamic_list_push(core->base->execution_states, (mqptr_t)(&st)) ==
        RET_FAILURE) {
      merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_CANNOT_CREATE_NEW_STATE_);
      return RET_FAILURE;
    }
    id = merry_dynamic_list_size(core->base->execution_states);
  } else {
    if (merry_dynamic_list_replace(core->base->execution_states, (mptr_t)(&st),
                                   id) == RET_FAILURE) {
      // This should not be executed since
      // it is valid.
      return RET_FAILURE;
    }
  }
  core->regr[ACC] = id;
  return RET_SUCCESS;
}

mret_t merry_64_bit_core_jmp_state(void *cptr, msize_t id) {
  merry_check_ptr(cptr);
  Merry64BitCore *core = (Merry64BitCore *)cptr;
  Merry64BitCoreState *st;
  if ((st = merry_dynamic_list_at(core->base->execution_states, id)) ==
      RET_NULL) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_INVALID_STATE_ID_);
    return RET_FAILURE;
  }

  core->ffregr = st->ffregr;
  core->fregr = st->fregr;
  core->pc = st->pc;
  for (msize_t i = 0; i < REG_COUNT_GPC_64; i++) {
    core->regr[i] = st->regr[i];
  }

  return RET_SUCCESS;
}

mret_t merry_64_bit_core_replace_state(void *cptr, msize_t id) {
  merry_check_ptr(cptr);
  Merry64BitCore *core = (Merry64BitCore *)cptr;
  Merry64BitCoreState *st;
  if ((st = merry_dynamic_list_at(core->base->execution_states, id)) ==
      RET_NULL) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_INVALID_STATE_ID_);
    return RET_FAILURE;
  }

  st->ffregr = core->ffregr;
  st->fregr = core->fregr;
  st->pc = core->pc;
  for (msize_t i = 0; i < REG_COUNT_GPC_64; i++) {
    st->regr[i] = core->regr[i];
  }

  return RET_SUCCESS;
}

mret_t merry_64_bit_core_del_state(void *cptr, msize_t id) {
  merry_check_ptr(cptr);
  Merry64BitCore *core = (Merry64BitCore *)cptr;
  Merry64BitCoreState *st;
  if ((st = merry_dynamic_list_at(core->base->execution_states, id)) ==
      RET_NULL) {
    merry_assign_state(core->base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_INVALID_STATE_ID_);
    return RET_FAILURE;
  }
  st->state_valid = mtrue; // free again

  return RET_SUCCESS;
}
