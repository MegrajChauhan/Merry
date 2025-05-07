#include "merry_core.h"
#include <merry_os.h>

MerryCore *merry_core_init(MerryMemory *inst_mem, MerryDMemory *data_mem,
                           msize_t id) {
  log("Initialzing VCore[ID: %lu]", id);
  // allocate a new core
  MerryCore *new_core = (MerryCore *)malloc(sizeof(MerryCore));
  // check if the core has been initialized
  if (new_core == RET_NULL) {
    mreport("Failed to allocate memory for VCore");
    return RET_NULL;
  }
  new_core->bp = 0; // initialize these registers to 0
  new_core->pc = 0;
  new_core->sp = (mqword_t)(-1);
  new_core->core_id = id;
  new_core->data_mem = data_mem;
  new_core->inst_mem = inst_mem;
  // initialize the locks and condition variables
  new_core->cond = merry_cond_init();
  if (new_core->cond == RET_NULL) {
    free(new_core);
    return RET_NULL;
  }
  new_core->lock = merry_mutex_init();
  if (new_core->lock == RET_NULL) {
    merry_cond_destroy(new_core->cond);
    free(new_core);
    return RET_NULL;
  }
  new_core->registers = (mqptr_t)malloc(sizeof(mqword_t) * REGR_COUNT);
  if (new_core->registers == RET_NULL)
    goto failure;
  merry_core_zero_out_reg(new_core);
  new_core->stack_mem = (mqptr_t)_MERRY_MEM_GET_PAGE_(
      _MERRY_STACKMEM_BYTE_LEN_, _MERRY_PROT_DEFAULT_, _MERRY_FLAG_DEFAULT_);
  if (new_core->stack_mem == RET_NULL) {
    mreport("Failed to allocate a stack[SYS ERR]");
    goto failure;
  }
#ifdef _USE_LINUX_
  madvise(new_core->stack_mem, _MERRY_STACKMEM_BYTE_LEN_, MADV_RANDOM);
  madvise(new_core->stack_mem, _MERRY_STACKMEM_BYTE_LEN_, MADV_WILLNEED);
#endif
  new_core->stop_running =
      mfalse; // this is set to false because as soon as the core is instructed
              // to start/continue execution, it shouldn't stop and start
              // immediately
  new_core->ras = merry_init_stack(_MERRY_RAS_LEN_, mtrue, _MERRY_RAS_LIMIT_,
                                   _MERRY_RAS_GROW_PER_RESIZE_);
  if (new_core->ras == RET_NULL)
    goto failure;
  // the same as the RAS
  new_core->callstack = merry_init_stack(
      _MERRY_RAS_LEN_, mtrue, _MERRY_RAS_LIMIT_, _MERRY_RAS_GROW_PER_RESIZE_);
  if (new_core->callstack == RET_NULL)
    goto failure;
  new_core->excp_set = mfalse;
  return new_core;
failure:
  merry_core_destroy(new_core, mtrue);
  return RET_NULL;
}

void merry_core_destroy(MerryCore *core, mbool_t _cond) {
  if (surelyF(core == NULL))
    return;
  if (_cond == mtrue)
    merry_cond_destroy(core->cond);
  merry_mutex_destroy(core->lock);
  if (surelyT(core->registers != NULL)) {
    free(core->registers);
  }
  if (surelyT(core->stack_mem != NULL)) {
    if (_MERRY_MEM_GIVE_PAGE_(core->stack_mem, _MERRY_STACKMEM_BYTE_LEN_) ==
        _MERRY_RET_GIVE_ERROR_) {
      // we failed here
      // This shouldn't happen unless the stack_mem variable was messed with and
      // corrupted This is the worst idea but we simply do nothing[OPPS UNSAFE?
      // PFTT...WHAT COULD POSSIBLY GO WRONG?!] nothing really, after the VM
      // terminates, the OS will unmap all the resources that was being used and
      // hence we become safe While it would be interesting to know why this
      // error happened, the cause probably didn't come from the VM itself since
      // i am trying to write it as "SAFE" as possible This may sound like a
      // joke considering how absurd, unstructured, redundand, unsafe the code
      // so far has been
    }
  }
  merry_destroy_stack(core->ras);
  merry_destroy_stack(core->callstack);
  core->data_mem = NULL;
  core->inst_mem = NULL;
  free(core);
}

_MERRY_INTERNAL_ mqword_t merry_core_get_immediate(MerryCore *core) {
  mqword_t res = 0;
  if (merry_manager_mem_read_inst(core->inst_mem, core->pc, &res) ==
      RET_FAILURE) {
    // we failed
    core->stop_running = mtrue;
    merry_requestHdlr_panic(core->inst_mem->error, core->core_id);
  }
  core->pc++;
  return res;
}

_MERRY_INTERNAL_ void merry_cmp_floats64(MerryCore *core, double val1,
                                         double val2) {
  if (val1 == val2)
    core->flag.zero = 1; // equal
  else
    core->flag.zero = 0;
  if (val1 > val2)
    core->greater = 1;
  else
    core->greater = 0;
  // only these two flags are affected
}

_MERRY_INTERNAL_ void merry_cmp_floats32(MerryCore *core, float val1,
                                         float val2) {
  if (val1 == val2)
    core->flag.zero = 1; // equal
  else
    core->flag.zero = 0;
  if (val1 > val2)
    core->greater = 1;
  else
    core->greater = 0;
  // only these two flags are affected
}

_THRET_T_ merry_runCore(mptr_t core) {
  MerryCore *c = (MerryCore *)core;
  log("VCore: %d[ACTIVE]", c->core_id);
  register mqptr_t current = &c->current_inst;
  register mqword_t curr = 0;
  F32 f32;
  F64 f64;
  while (mtrue) {
    // this will be false most of the times so makes sense
    if (surelyF(c->stop_running == mtrue))
      break;
    if (merry_manager_mem_read_inst(c->inst_mem, c->pc, current) ==
        RET_FAILURE) {
      merry_requestHdlr_panic(c->inst_mem->error, c->core_id);
      break; // stay out of it
    }
    c->pc++;
    curr = *current;
    switch (merry_get_opcode(*current)) {
    case OP_NOP: // we don't care about NOP instructions
      break;
    case OP_HALT: // Simply stop the core
      merry_requestHdlr_push_request(_REQ_REQHALT, c->core_id, c->cond);
      c->stop_running = mtrue;
      break;
    case OP_ADD_IMM:
      merry_execute_add_imm(c);
      break;
    case OP_ADD_REG:
      merry_execute_add_reg(c);
      break;
    case OP_SUB_IMM:
      merry_execute_sub_imm(c);
      break;
    case OP_SUB_REG:
      merry_execute_sub_reg(c);
      break;
    case OP_MUL_IMM:
      merry_execute_mul_imm(c);
      break;
    case OP_MUL_REG:
      merry_execute_mul_reg(c);
      break;
    case OP_DIV_IMM:
      merry_execute_div_imm(c);
      break;
    case OP_DIV_REG:
      merry_execute_div_reg(c);
      break;
    case OP_MOD_IMM:
      merry_execute_mod_imm(c);
      break;
    case OP_MOD_REG:
      merry_execute_mod_reg(c);
      break;
    case OP_IADD_IMM:
      merry_execute_iadd_imm(c);
      break;
    case OP_IADD_REG:
      merry_execute_iadd_reg(c);
      break;
    case OP_ISUB_IMM:
      merry_execute_isub_imm(c);
      break;
    case OP_ISUB_REG:
      merry_execute_isub_reg(c);
      break;
    case OP_IMUL_IMM:
      merry_execute_imul_imm(c);
      break;
    case OP_IMUL_REG:
      merry_execute_imul_reg(c);
      break;
    case OP_IDIV_IMM:
      merry_execute_idiv_imm(c);
      break;
    case OP_IDIV_REG:
      merry_execute_idiv_reg(c);
      break;
    case OP_IMOD_IMM:
      merry_execute_imod_imm(c);
      break;
    case OP_IMOD_REG:
      merry_execute_imod_reg(c);
      break;
    case OP_FADD:
      merry_execute_fadd(c);
      break;
    case OP_FSUB:
      merry_execute_fsub(c);
      break;
    case OP_FMUL:
      merry_execute_fmul(c);
      break;
    case OP_FDIV:
      merry_execute_fdiv(c);
      break;
    case OP_FADD32:
      merry_execute_fadd32(c);
      break;
    case OP_FSUB32:
      merry_execute_fsub32(c);
      break;
    case OP_FMUL32:
      merry_execute_fmul32(c);
      break;
    case OP_FDIV32:
      merry_execute_fdiv32(c);
      break;
    case OP_MOVE_IMM: // just 32 bits immediates
      c->registers[(curr >> 48) & 15] = (curr) & 0xFFFFFFFF;
      break;
    case OP_MOVNZ:
    case OP_MOVNE:
      if (c->flag.zero == 0)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVE:
    case OP_MOVZ:
      if (c->flag.zero == 1)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVNC:
      if (c->flag.carry == 0)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVC:
      if (c->flag.carry == 1)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVNO:
      if (c->flag.overflow == 0)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVO:
      if (c->flag.overflow == 1)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVNN:
      if (c->flag.negative == 0)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVN:
      if (c->flag.negative == 1)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVS:
    case OP_MOVNG:
      if (c->greater == 0)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVNS:
    case OP_MOVG:
      if (c->greater == 1)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVGE:
      if (c->greater == 1 || c->flag.zero == 1)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVSE:
      if (c->greater == 0 || c->flag.zero == 1)
        c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVE_IMM_64: // 64 bits immediates
      c->registers[*current & 15] = merry_core_get_immediate(c);
      break;
    case OP_MOVE_REG: // moves the whole 8 bytes
      c->registers[(curr >> 4) & 15] = c->registers[curr & 15]; // this is all
      break;
    case OP_MOVE_REG8: // moves only the lowest byte
      c->registers[(curr >> 4) & 15] = c->registers[curr & 15] & 0xFF;
      break;
    case OP_MOVE_REG16: // moves only the lowest 2 bytes
      c->registers[(curr >> 4) & 15] = c->registers[curr & 15] & 0xFFFF;
      break;
    case OP_MOVE_REG32: // moves only the lowest 4 byte
      c->registers[(curr >> 4) & 15] = c->registers[curr & 15] & 0xFFFFFF;
      break;
    case OP_MOVESX_IMM8:
      merry_execute_movesx_imm8(c);
      break;
    case OP_MOVESX_IMM16:
      merry_execute_movesx_imm16(c);
      break;
    case OP_MOVESX_IMM32:
      merry_execute_movesx_imm32(c);
      break;
    case OP_MOVESX_REG8:
      merry_execute_movesx_reg8(c);
      break;
    case OP_MOVESX_REG16:
      merry_execute_movesx_reg16(c);
      break;
    case OP_MOVESX_REG32:
      merry_execute_movesx_reg32(c);
      break;
    case OP_JMP_OFF: // we have to make the 5 bytes 8 bytes by sign extension in
                     // case it is indeed in 2's complement the offset in this
                     // case is a signed number and the 40th bit is the sign bit
                     // the decoder executes the jump instruction
      register mqword_t off = (*current & 0xFFFFFFFFFFFF);
      if ((off >> 47) == 1)
        off |= 0xFFFF000000000000;
      c->pc += off;
      break;
    case OP_JMP_REGR:
      *current |= (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_JMP_ADDR:
      // 6 bytes should be fine
      c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_CALL:
      // save the current return address
      register mqword_t addr = *current & 0xFFFFFFFFFFFF;
      if (merry_stack_push(c->ras, c->pc) == RET_FAILURE) {
        merry_requestHdlr_panic(MERRY_CALL_DEPTH_REACHED, c->core_id);
        c->stop_running = mtrue;
        break;
      }
      if (merry_stack_push(c->callstack, addr) == RET_FAILURE) {
        merry_requestHdlr_panic(MERRY_INTERNAL_ERROR, c->core_id);
        c->stop_running = mtrue;
        break;
      }
      c->pc = addr; // the address to the first instruction of the procedure
      merry_execute_call(c);
      break;
    case OP_CALL_REG:
      // save the current return address
      addr = c->registers[(*current & 15)];
      if (merry_stack_push(c->ras, c->pc) == RET_FAILURE) {
        merry_requestHdlr_panic(MERRY_CALL_DEPTH_REACHED, c->core_id);
        c->stop_running = mtrue;
        break;
      }
      if (merry_stack_push(c->callstack, addr) == RET_FAILURE) {
        merry_requestHdlr_panic(MERRY_INTERNAL_ERROR, c->core_id);
        c->stop_running = mtrue;
        break;
      }
      c->pc = addr; // the address to the first instruction of the procedure
      merry_execute_call(c);
      break;
    case OP_RETZ:
    case OP_RETE:
      if (c->flag.zero == 1)
        goto _ret;
      break;
    case OP_RETNZ:
    case OP_RETNE:
      if (c->flag.zero == 0)
        goto _ret;
      break;
    case OP_RETNC:
      if (c->flag.carry == 0)
        goto _ret;
      break;
    case OP_RETC:
      if (c->flag.carry == 1)
        goto _ret;
      break;
    case OP_RETNO:
      if (c->flag.overflow == 0)
        goto _ret;
      break;
    case OP_RETO:
      if (c->flag.overflow == 1)
        goto _ret;
      break;
    case OP_RETNN:
      if (c->flag.negative == 0)
        goto _ret;
      break;
    case OP_RETN:
      if (c->flag.negative == 1)
        goto _ret;
      break;
    case OP_RETS:
    case OP_RETNG:
      if (c->greater == 0)
        goto _ret;
      break;
    case OP_RETNS:
    case OP_RETG:
      if (c->greater == 1)
        goto _ret;
      break;
    case OP_RETGE:
      if (c->greater == 1 || c->flag.zero == 1)
        goto _ret;
      break;
    case OP_RETSE:
      if (c->greater == 0 || c->flag.zero == 1)
        goto _ret;
      break;
    case OP_RET:
    // we just have to pop the topmost
    _ret:
      mqword_t a = 0;
      if (merry_stack_pop(c->ras, &c->pc) == RET_FAILURE) {
        merry_requestHdlr_panic(MERRY_INVALID_RETURN, c->core_id);
        c->stop_running = mtrue;
        break;
      }
      if (merry_stack_pop(c->callstack, &a) == RET_FAILURE) {
        merry_requestHdlr_panic(MERRY_INTERNAL_ERROR, c->core_id);
        c->stop_running = mtrue;
        break;
      }
      // pc should have been restored
      merry_execute_ret(c);
      break;
    case OP_SVA: // [SVA stands for Stack Variable Access]
      merry_execute_sva(c);
      break;
    case OP_SVA_REG:
      c->current_inst |= (c->registers[*current & 15] & 0xFFFF);
      merry_execute_sva(c);
      break;
    case OP_SVC: // [SVC stands for Stack Variable Change]
      merry_execute_svc(c);
      break;
    case OP_SVC_REG:
      c->current_inst |= (c->registers[*current & 15] & 0xFFFF);
      merry_execute_svc(c);
      break;
    case OP_GSS_IMM:
      merry_execute_gss_imm(c);
      break;
    case OP_GSS_REG:
      c->current_inst |= (c->registers[*current & 15] & 0xFFFF);
      merry_execute_gss_imm(c);
      break;
    case OP_SSS_IMM:
      merry_execute_sss_imm(c);
      break;
    case OP_SSS_REG:
      c->current_inst |= (c->registers[*current & 15] & 0xFFFF);
      merry_execute_sss_imm(c);
      break;
    case OP_PUSH_IMM:
      merry_execute_push_imm(c);
      break;
    case OP_PUSH_REG:
      merry_execute_push_reg(c);
      break;
    case OP_POP:
      merry_execute_pop(c);
      break;
    case OP_PUSHA:
      merry_execute_pusha(c);
      break;
    case OP_POPA:
      merry_execute_popa(c);
      break;
    case OP_AND_IMM:
      c->registers[*current & 15] &= merry_core_get_immediate(c);
      break;
    case OP_AND_REG:
      curr = *current;
      c->registers[(curr >> 4) & 15] &= c->registers[curr & 15];
      break;
    case OP_OR_IMM:
      c->registers[*current & 15] |= merry_core_get_immediate(c);
      break;
    case OP_OR_REG:
      curr = *current;
      c->registers[(curr >> 4) & 15] |= c->registers[curr & 15];
      break;
    case OP_XOR_IMM:
      c->registers[*current & 15] ^= merry_core_get_immediate(c);
      break;
    case OP_XOR_REG:
      curr = *current;
      c->registers[(curr >> 4) & 15] ^= c->registers[curr & 15];
      break;
    case OP_NOT:
      curr = *current & 15;
      c->registers[curr] = ~c->registers[curr];
      break;
    case OP_LSHIFT:
      curr = *current;
      c->registers[(curr >> 8) & 15] <<= curr & 0x40;
      break;
    case OP_LSHIFT_REGR:
      curr = *current;
      c->registers[(curr >> 4) & 15] <<= c->registers[curr & 15] & 0x40;
      break;
    case OP_RSHIFT:
      curr = *current;
      c->registers[(curr >> 8) & 15] >>= curr & 0x40;
      break;
    case OP_RSHIFT_REGR:
      curr = *current;
      c->registers[(curr >> 4) & 15] >>= c->registers[curr & 15] & 0x40;
      break;
    case OP_CMP_IMM:
      register mqword_t reg = c->registers[*current & 15];
      register mqword_t imm = merry_core_get_immediate(c);
      _cmp_inst_(reg, imm, &c->flag);
      if (reg > imm)
        c->greater = 1;
      else
        c->greater = 0;
      break;
    case OP_CMP_IMM_MEMB:
      merry_execute_cmp_mem(c, merry_dmemory_read_byte);
      break;
    case OP_CMP_IMM_MEMW:
      merry_execute_cmp_mem(c, merry_dmemory_read_word);
      break;
    case OP_CMP_IMM_MEMD:
      merry_execute_cmp_mem(c, merry_dmemory_read_dword);
      break;
    case OP_CMP_IMM_MEMQ:
      merry_execute_cmp_mem(c, merry_dmemory_read_qword);
      break;
    case OP_CMP_REG:
      register mqword_t reg1 = c->registers[(*current >> 4) & 15];
      register mqword_t reg2 = c->registers[*current & 15];
      _cmp_inst_(reg1, reg2, &c->flag);
      if (reg1 > reg2)
        c->greater = 1;
      break;
    case OP_INC:
      c->registers[*current & 15]++;
      break;
    case OP_DEC:
      c->registers[*current & 15]--; // now inc and dec are able to affect the
                                     // flags register?
      break;
    case OP_LEA:
      // 000000000 0000000 00000000 00000000 00000000 00000000 00000000 00000000
      curr = *current;
      c->registers[(curr >> 24) & 15] =
          c->registers[(curr >> 16) & 15] +
          c->registers[(curr >> 8) & 15] * c->registers[curr & 15];
      break;
    case OP_LOAD:
      merry_execute_load(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_STORE:
      merry_execute_store(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_LOADB:
      merry_execute_loadb(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_STOREB:
      merry_execute_storeb(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_LOADW:
      merry_execute_loadw(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_STOREW:
      merry_execute_storew(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_LOADD:
      merry_execute_loadd(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_STORED:
      merry_execute_stored(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_ATOMIC_LOAD_REG:
      *current |= (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_ATOMIC_LOAD:
      merry_execute_atm_load(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_ATOMIC_STORE_REG:
      *current |= (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_ATOMIC_STORE:
      merry_execute_atm_store(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_ATOMIC_LOADB_REG:
      *current |= (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_ATOMIC_LOADB:
      merry_execute_atm_loadb(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_ATOMIC_STOREB_REG:
      *current |= (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_ATOMIC_STOREB:
      merry_execute_atm_storeb(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_ATOMIC_LOADW_REG:
      *current |= (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_ATOMIC_LOADW:
      merry_execute_atm_loadw(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_ATOMIC_STOREW_REG:
      *current |= (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_ATOMIC_STOREW:
      merry_execute_atm_storew(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_ATOMIC_LOADD_REG:
      *current |= (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_ATOMIC_LOADD:
      merry_execute_atm_loadd(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_ATOMIC_STORED_REG:
      *current |= (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_ATOMIC_STORED:
      merry_execute_atm_stored(c, *current & 0xFFFFFFFFFFFF);
      break;
    case OP_LOAD_REG:
      merry_execute_load_reg(c, c->registers[((*current) & 15)]);
      break;
    case OP_STORE_REG:
      merry_execute_store_reg(c, c->registers[((*current) & 15)]);
      break;
    case OP_LOADB_REG:
      merry_execute_loadb_reg(c, c->registers[((*current) & 15)]);
      break;
    case OP_STOREB_REG:
      merry_execute_storeb_reg(c, c->registers[((*current) & 15)]);
      break;
    case OP_LOADW_REG:
      merry_execute_loadw_reg(c, c->registers[((*current) & 15)]);
      break;
    case OP_STOREW_REG:
      merry_execute_storew_reg(c, c->registers[((*current) & 15)]);
      break;
    case OP_LOADD_REG:
      merry_execute_loadd_reg(c, c->registers[((*current) & 15)]);
      break;
    case OP_STORED_REG:
      merry_execute_stored_reg(c, c->registers[((*current) & 15)]);
      break;
    case OP_EXCG8:
      merry_execute_excg8(c);
      break;
    case OP_EXCG16:
      merry_execute_excg16(c);
      break;
    case OP_EXCG32:
      merry_execute_excg32(c);
      break;
    case OP_EXCG:
      merry_execute_excg(c);
      break;
    case OP_MOV8:
      curr = *current;
      c->registers[(curr >> 4) & 15] &=
          (0xFFFFFFFFFFFFFF00 | (c->registers[curr & 15] & 0xFF));
      break;
    case OP_MOV16:
      curr = *current;
      c->registers[(curr >> 4) & 15] &=
          (0xFFFFFFFFFFFF0000 | (c->registers[curr & 15] & 0xFFFF));
      break;
    case OP_MOV32:
      curr = *current;
      c->registers[(curr >> 4) & 15] &=
          (0xFFFFFFFFFF000000 | (c->registers[curr & 15] & 0xFFFFFF));
      break;
    case OP_CFLAGS:
      c->flag.carry = 0;
      c->flag.negative = 0;
      c->flag.overflow = 0;
      c->flag.zero = 0;
      c->greater = 0;
      break;
    case OP_RESET:
      merry_core_zero_out_reg(c);
      break;
    case OP_CLC:
      _fclear_(carry);
      break;
    case OP_CLZ:
      _fclear_(zero);
      break;
    case OP_CLN:
      _fclear_(negative);
      break;
    case OP_CLO:
      _fclear_(overflow);
      break;
    case OP_JZ:
    case OP_JE:
      // the address to jmp should follow the instruction
      if (c->flag.zero == 1)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JNZ:
    case OP_JNE:
      // the address to jmp should follow the instruction
      if (c->flag.zero == 0)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JNC:
      if (c->flag.carry == 0)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JC:
      if (c->flag.carry == 1)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JNO:
      if (c->flag.overflow == 0)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JO:
      if (c->flag.overflow == 1)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JNN:
      if (c->flag.negative == 0)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JN:
      if (c->flag.negative == 1)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JS:
    case OP_JNG:
      if (c->greater == 0)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JNS:
    case OP_JG:
      if (c->greater == 1)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JGE:
      if (c->greater == 1 || c->flag.zero == 0)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_JSE:
      if (c->greater == 0 || c->flag.zero == 0)
        c->pc = (*current & 0xFFFFFFFFFFFF);
      break;
    case OP_LOOP:
      if (c->registers[Mc] != 0) {
        c->pc = (*current & 0xFFFFFFFFFFFF);
        c->registers[Mc]--;
      }
      break;
    case OP_INTR:
      if (merry_requestHdlr_push_request(*current & 0xFFFF, c->core_id,
                                         c->cond) == RET_FAILURE)
        c->stop_running = mtrue;
      break;
    case OP_CMPXCHG_REGR:
      register mqword_t v = c->registers[*current & 15];
      *current &= 0xFFFFFFFFFFFFFF00;
      *current |= (v & 0xFFFFFFFFFFFF);
    case OP_CMPXCHG:
      // this operation must be atomic
      // but it cannot be guranteed in a VM
      // this instruction will take a 6-byte address and 2 registers
      // this works for 1 byte only
      {
        mbptr_t _addr_ = merry_dmemory_get_byte_address(
            c->data_mem, *current & 0xFFFFFFFFFFFF);
        mbyte_t val = (c->registers[(*current >> 52) & 15]) & 0xFF;
        if (_addr_ == RET_NULL) {
          merry_requestHdlr_panic(c->data_mem->error, c->core_id);
          c->stop_running = mtrue;
          break;
        }
        c->flag.zero = atomic_compare_exchange_strong(
            _addr_, &val, c->registers[(*current >> 48) & 15] & 0xFF);
      }
      break;
    case OP_CIN:
      // the input is stored in a register that is encoded into the last 4 bits
      // of the instruction
      c->registers[*current & 15] = getchar();
      break;
    case OP_COUT:
      // the byte to output is stored in a register that is encoded into the
      // last 4 bits of the instruction
      putchar((int)c->registers[*current & 15]);
      break;
    case OP_SIN_REG:
      *current = (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_SIN:
      // the address to store in is encoded into the instruction
      // the number of bytes to input is in the Mc register
      {
        register mqword_t len = c->registers[Mc];
        mbptr_t temp = (mbptr_t)malloc(len);
        if (temp == NULL) {
          merry_requestHdlr_panic(MERRY_INTERNAL_ERROR, c->core_id);
          c->stop_running = mtrue;
          break;
        }
        for (msize_t i = 0; i < len; i++) {
          temp[i] = getchar();
        }
        if (merry_dmemory_write_bytes_maybe_over_multiple_pages(
                c->data_mem, *current & 0xFFFFFFFFFFFF, len, temp) ==
            RET_FAILURE) {
          merry_requestHdlr_panic(MERRY_INTERNAL_ERROR, c->core_id);
          c->stop_running = mtrue;
        }
        free(temp);
      }
      break;
    case OP_SOUT_REG:
      *current = (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_SOUT:
      // the address to store in is encoded into the instruction
      // the number of bytes to output is in the Mc register
      {
        register mqword_t len = c->registers[Mc];
        mstr_t str = merry_dmemory_get_bytes_maybe_over_multiple_pages(
            c->data_mem, *current & 0xFFFFFFFFFFFF, len);
        if (str == RET_NULL) {
          merry_requestHdlr_panic(c->data_mem->error, c->core_id);
          c->stop_running = mtrue;
          break;
        }
        printf("%s", str);
        free(str);
      }
      break;
    case OP_IN:
      fscanf(stdin, "%hhi", &c->registers[*current & 15]);
      break;
    case OP_OUT:
      fprintf(stdout, "%hhi", c->registers[*current & 15]);
      break;
    case OP_INW:
      // same as OP_IN, store in a register
      fscanf(stdin, "%hd", &c->registers[*current & 15]);
      break;
    case OP_OUTW:
      // same as OP_OUT, stored in a register
      fprintf(stdout, "%hd", c->registers[*current & 15]);
      break;
    case OP_IND:
      fscanf(stdin, "%d", &c->registers[*current & 15]);
      break;
    case OP_OUTD:
      fprintf(stdout, "%d", c->registers[*current & 15]);
      break;
    case OP_INQ:
      fscanf(stdin, "%lld", &c->registers[*current & 15]);
      break;
    case OP_OUTQ:
      fprintf(stdout, "%lld", c->registers[*current & 15]);
      break;
    case OP_UIN:
      fscanf(stdin, "%hhu", &c->registers[*current & 15]);
      break;
    case OP_UOUT:
      fprintf(stdout, "%hhu", c->registers[*current & 15]);
      break;
    case OP_UINW:
      // same as OP_IN, store in a register
      fscanf(stdin, "%hu", &c->registers[*current & 15]);
      break;
    case OP_UOUTW:
      // same as OP_OUT, stored in a register
      fprintf(stdout, "%hu", c->registers[*current & 15]);
      break;
    case OP_UIND:
      fscanf(stdin, "%d", &c->registers[*current & 15]);
      break;
    case OP_UOUTD:
      fprintf(stdout, "%u", c->registers[*current & 15]);
      break;
    case OP_UINQ:
      fscanf(stdin, "%llu", &c->registers[*current & 15]);
      break;
    case OP_UOUTQ:
      fprintf(stdout, "%llu", c->registers[*current & 15]);
      break;
    case OP_INF:
      fscanf(stdin, "%lf", &f64._double);
      c->registers[*current & 15] = f64._integer;
      break;
    case OP_OUTF:
      f64._integer = c->registers[*current & 15];
      fprintf(stdout, "%lf", f64._double);
      break;
    case OP_INF32:
      fscanf(stdin, "%f", &f32._float);
      c->registers[*current & 15] = f32._integer;
      break;
    case OP_OUTF32:
      f32._integer = c->registers[*current & 15];
      fprintf(stdout, "%f", f32._float);
      break;
    case OP_OUTR:
      for (msize_t i = 0; i < REGR_COUNT; i++)
        fprintf(stdout, "%lli\n", c->registers[i]);
      break;
    case OP_UOUTR:
      for (msize_t i = 0; i < REGR_COUNT; i++)
        fprintf(stdout, "%llu\n", c->registers[i]);
      break;
    case OP_ADD_MEMB:
      merry_execute_add_mem(c, &merry_dmemory_read_byte);
      break;
    case OP_ADD_MEMW:
      merry_execute_add_mem(c, &merry_dmemory_read_word);
      break;
    case OP_ADD_MEMD:
      merry_execute_add_mem(c, &merry_dmemory_read_dword);
      break;
    case OP_ADD_MEMQ:
      merry_execute_add_mem(c, &merry_dmemory_read_qword);
      break;
    case OP_SUB_MEMB:
      merry_execute_sub_mem(c, &merry_dmemory_read_byte);
      break;
    case OP_SUB_MEMW:
      merry_execute_sub_mem(c, &merry_dmemory_read_word);
      break;
    case OP_SUB_MEMD:
      merry_execute_sub_mem(c, &merry_dmemory_read_dword);
      break;
    case OP_SUB_MEMQ:
      merry_execute_sub_mem(c, &merry_dmemory_read_qword);
      break;
    case OP_MUL_MEMB:
      merry_execute_mul_mem(c, &merry_dmemory_read_byte);
      break;
    case OP_MUL_MEMW:
      merry_execute_mul_mem(c, &merry_dmemory_read_word);
      break;
    case OP_MUL_MEMD:
      merry_execute_mul_mem(c, &merry_dmemory_read_dword);
      break;
    case OP_MUL_MEMQ:
      merry_execute_mul_mem(c, &merry_dmemory_read_qword);
      break;
    case OP_DIV_MEMB:
      merry_execute_div_mem(c, &merry_dmemory_read_byte);
      break;
    case OP_DIV_MEMW:
      merry_execute_div_mem(c, &merry_dmemory_read_word);
      break;
    case OP_DIV_MEMD:
      merry_execute_div_mem(c, &merry_dmemory_read_dword);
      break;
    case OP_DIV_MEMQ:
      merry_execute_div_mem(c, &merry_dmemory_read_qword);
      break;
    case OP_MOD_MEMB:
      merry_execute_mod_mem(c, &merry_dmemory_read_byte);
      break;
    case OP_MOD_MEMW:
      merry_execute_mod_mem(c, &merry_dmemory_read_word);
      break;
    case OP_MOD_MEMD:
      merry_execute_mod_mem(c, &merry_dmemory_read_dword);
      break;
    case OP_MOD_MEMQ:
      merry_execute_mod_mem(c, &merry_dmemory_read_qword);
      break;
    case OP_FADD32_MEM:
      merry_execute_fadd32_mem(c);
      break;
    case OP_FADD_MEM:
      merry_execute_fadd64_mem(c);
      break;
    case OP_FSUB32_MEM:
      merry_execute_fsub32_mem(c);
      break;
    case OP_FSUB_MEM:
      merry_execute_fsub64_mem(c);
      break;
    case OP_FMUL32_MEM:
      merry_execute_fmul32_mem(c);
      break;
    case OP_FMUL_MEM:
      merry_execute_fmul64_mem(c);
      break;
    case OP_FDIV32_MEM:
      merry_execute_fdiv32_mem(c);
      break;
    case OP_FDIV_MEM:
      merry_execute_fdiv64_mem(c);
      break;
    case OP_SET_EXCP: // save an address to jump to when errors are encountered
      c->excp_set = mtrue;
      c->exception_address = *current & 0xFFFFFFFFFFFF;
      break;
    case OP_CALL_EXCP: // jump to the set exception handler
      if (c->excp_set == mfalse)
        break;
      if (merry_stack_push(c->ras, c->pc) == RET_FAILURE) {
        merry_requestHdlr_panic(MERRY_CALL_DEPTH_REACHED, c->core_id);
        c->stop_running = mtrue;
        break;
      }
      c->pc = c->exception_address; // the address to the first instruction of
                                    // the procedure
      merry_execute_call(c);
      break;
    case OP_SVA_MEM:
      merry_execute_sva_mem(c);
      break;
    case OP_SVC_MEM:
      merry_execute_svc_mem(c);
      break;
    case OP_SYSCALL:
      merry_requestHdlr_push_request(_REQ_SYSCALL, c->core_id, c->cond);
      break;
    case OP_PUSH_MEMB:
      merry_execute_push_mem(c, &merry_dmemory_read_byte);
      break;
    case OP_PUSH_MEMW:
      merry_execute_push_mem(c, &merry_dmemory_read_word);
      break;
    case OP_PUSH_MEMD:
      merry_execute_push_mem(c, &merry_dmemory_read_dword);
      break;
    case OP_PUSH_MEMQ:
      merry_execute_push_mem(c, &merry_dmemory_read_qword);
      break;
    case OP_POP_MEMB:
      merry_execute_pop_mem(c, &merry_dmemory_write_byte);
      break;
    case OP_POP_MEMW:
      merry_execute_pop_mem(c, &merry_dmemory_write_word);
      break;
    case OP_POP_MEMD:
      merry_execute_pop_mem(c, &merry_dmemory_write_dword);
      break;
    case OP_POP_MEMQ:
      merry_execute_pop_mem(c, &merry_dmemory_write_qword);
      break;
    case OP_GET_ADDR_REG:
      *current |= (c->registers[*current & 15] & 0xFFFFFFFFFFFF);
    case OP_GET_ADDR: {
      // get the physical address of some byte(You cannot do anything with this
      // other than for syscalls) The threat remains from passing it to
      // dynamically loaded libraries though
      register mbptr_t addr = merry_dmemory_get_byte_address(
          c->data_mem, *current & 0xFFFFFFFFFFFF);
      if (addr == NULL) {
        merry_requestHdlr_panic(c->data_mem->error, c->core_id);
        c->stop_running = mtrue;
        break;
      }
      c->registers[(*current >> 48) & 15] =
          (msize_t)addr; // storing the address
      break;
    }
    case OP_FCMP32: {
      register F32 f1, f2;
      f1._integer = c->registers[(*current >> 4) & 15];
      f2._integer = c->registers[(*current) & 15];
      merry_cmp_floats32(c, f1._float, f2._float);
      break;
    }
    case OP_FCMP: {
      register F64 f1, f2;
      f1._integer = c->registers[(*current >> 4) & 15];
      f2._integer = c->registers[(*current) & 15];
      merry_cmp_floats64(c, f1._double, f2._double);
      break;
    }
    }
  }
#if defined(_MERRY_HOST_OS_LINUX_)
  return (mptr_t)c->registers[Ma];
#elif defined(_MERRY_HOST_OS_WINDOWS_)
  return c->registers[Ma];
#endif
}

_MERRY_ALWAYS_INLINE_ inline _exec_(add_imm) {
  // add immediate value to a register
  _ArithMeticImmFrame_(+);
}

_MERRY_ALWAYS_INLINE_ inline _exec_(add_reg) {
  // add one register to another register
  _ArithMeticRegFrame_(+);
}

_MERRY_ALWAYS_INLINE_ inline _exec_(sub_imm) {
  _ArithMeticImmFrame_(-) if (core->flag.negative == 0) core->greater = 1;
}

_MERRY_ALWAYS_INLINE_ inline _exec_(sub_reg) {
  _ArithMeticRegFrame_(-) if (core->flag.negative == 0) core->greater = 1;
}

_MERRY_ALWAYS_INLINE_ inline _exec_(mul_imm) { _ArithMeticImmFrame_(*); }

_MERRY_ALWAYS_INLINE_ inline _exec_(mul_reg) { _ArithMeticRegFrame_(*); }

_exec_(div_imm) {
  // We need to add checks here to check if the operands are valid or not
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t imm = current & 0xFFFFFFFF;
  if (imm == 0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  core->registers[reg] = core->registers[reg] / imm;
  _update_flags_(&core->flag);
}

_exec_(div_reg) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  if (core->registers[reg2] == 0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] = core->registers[reg] / core->registers[reg2];
  _update_flags_(&core->flag);
}

_exec_(mod_imm) {
  // We need to add checks here to check if the operands are valid or not
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t imm = current & 0xFFFFFFFF;
  if (imm == 0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  core->registers[reg] = core->registers[reg] % imm;
  _update_flags_(&core->flag);
}

_exec_(mod_reg) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 52) & 15;
  register mqword_t reg2 = (current >> 48) & 15;
  if (core->registers[reg2] == 0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] = core->registers[reg] % core->registers[reg2];
  _update_flags_(&core->flag);
}

_lexec_(add_mem, mem_read func) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (func(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] += temp;
  _update_flags_(&core->flag);
}

_exec_(fadd32_mem) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_dmemory_read_dword(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] =
      ((float)core->registers[(current >> 48) & 15] + (float)temp);
  merry_cmp_floats32(core, (float)core->registers[reg], (float)temp);
}

_exec_(fadd64_mem) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_dmemory_read_qword(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] = ((double)core->registers[reg] + (double)temp);
  merry_cmp_floats32(core, (double)core->registers[reg], (double)temp);
}

_lexec_(sub_mem, mem_read func) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (func(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] -= temp;
  _update_flags_(&core->flag);
}

_exec_(fsub32_mem) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_dmemory_read_dword(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] = ((float)core->registers[reg] - (float)temp);
  merry_cmp_floats32(core, (float)core->registers[reg], (float)temp);
}

_exec_(fsub64_mem) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_dmemory_read_qword(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] = ((double)core->registers[reg] - (double)temp);
  merry_cmp_floats64(core, (double)core->registers[reg], (double)temp);
}

_lexec_(mul_mem, mem_read func) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (func(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] *= temp;
  _update_flags_(&core->flag);
}

_exec_(fmul32_mem) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_dmemory_read_dword(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] = ((float)core->registers[reg] * (float)temp);
  merry_cmp_floats32(core, (float)core->registers[reg], (float)temp);
}

_exec_(fmul64_mem) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_dmemory_read_qword(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] = ((double)core->registers[reg] * (double)temp);
  merry_cmp_floats64(core, (double)core->registers[reg], (double)temp);
}

_lexec_(div_mem, mem_read func) {
  register mqword_t current = core->current_inst;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (func(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  if (temp == 0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[(current >> 48) & 15] /= temp;
  _update_flags_(&core->flag);
}

_exec_(fdiv32_mem) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_dmemory_read_dword(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  if ((float)temp == 0.0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] =
      ((float)core->registers[(current >> 48) & 15] * (float)temp);
  merry_cmp_floats32(core, (float)core->registers[reg], (float)temp);
}

_exec_(fdiv64_mem) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (merry_dmemory_read_qword(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  if ((double)temp == 0.0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] = ((double)core->registers[reg] * (double)temp);
  merry_cmp_floats64(core, (double)core->registers[reg], (double)temp);
}

_lexec_(mod_mem, mem_read func) {
  register mqword_t current = core->current_inst;
  register mqword_t addr = (current & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (func(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  if (temp == 0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[(current >> 48) & 15] %= temp;
  _update_flags_(&core->flag);
}

_lexec_(cmp_mem, mem_read func) {
  register mqword_t reg = core->registers[(core->current_inst >> 48) & 15];
  register mqword_t addr = (core->current_inst & 0xFFFFFFFFFFFF);
  mqword_t temp = 0;
  if (func(core->data_mem, addr, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  _cmp_inst_(reg, temp, &core->flag);
  if (reg > temp)
    core->greater = 1;
  else
    core->greater = 0;
}

_exec_(iadd_imm) {
  // The processor will treat op1 and op2 as signed values
  // Since we will get a result that is also signed, we don't have to worry
  // about anything
  _SArithMeticImmFrame_(+);
}

_exec_(iadd_reg) { _SArithMeticRegFrame_(+); }

_exec_(isub_imm) {
  _SArithMeticImmFrame_(-);
  if (core->flag.negative == 0)
    core->greater = 1;
}

_exec_(isub_reg) {
  _SArithMeticRegFrame_(-);
  if (core->flag.negative == 0)
    core->greater = 1;
}

_exec_(imul_imm) { _SArithMeticImmFrame_(*); }

_exec_(imul_reg) { _SArithMeticRegFrame_(*); }

_exec_(idiv_imm) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t imm = current & _sign_extend32_(0xFFFFFFFF);
  if (imm == 0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  core->registers[reg] = core->registers[reg] / imm;
  _update_flags_(&core->flag);
}

_exec_(idiv_reg) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  if (core->registers[reg2] == 0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] = core->registers[reg] / core->registers[reg2];
  _update_flags_(&core->flag);
}

_exec_(imod_imm) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 48) & 15;
  register mqword_t imm = current & _sign_extend32_(0xFFFFFFFF);
  if (imm == 0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  core->registers[reg] = core->registers[reg] % imm;
  _update_flags_(&core->flag);
}

_exec_(imod_reg) {
  register mqword_t current = core->current_inst;
  register mqword_t reg = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  if (core->registers[reg2] == 0) {
    merry_requestHdlr_panic(MERRY_DIV_BY_ZERO, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[reg] = core->registers[reg] % core->registers[reg2];
  _update_flags_(&core->flag);
}

_exec_(fadd) {
  register mqword_t current = core->current_inst;
  register mqword_t reg1 = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  core->registers[reg1] =
      ((double)core->registers[reg1] + (double)core->registers[reg2]);
  // we have no flags for floating point arithmetic
  // so we gotta do with the flags register we have
  merry_cmp_floats64(core, core->registers[reg1], core->registers[reg2]);
}

_exec_(fsub) {
  register mqword_t current = core->current_inst;
  register mqword_t reg1 = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  core->registers[reg1] =
      ((double)core->registers[reg1] - (double)core->registers[reg2]);
  merry_cmp_floats64(core, core->registers[reg1], core->registers[reg2]);
}

_exec_(fmul) {
  register mqword_t current = core->current_inst;
  register mqword_t reg1 = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  core->registers[reg1] =
      ((double)core->registers[reg1] * (double)core->registers[reg2]);
  merry_cmp_floats64(core, core->registers[reg1], core->registers[reg2]);
}

_exec_(fdiv) {
  register mqword_t current = core->current_inst;
  register mqword_t reg1 = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  core->registers[reg1] =
      ((double)core->registers[reg1] / (double)core->registers[reg2]);
  merry_cmp_floats64(core, core->registers[reg1], core->registers[reg2]);
}

_exec_(fadd32) {
  register mqword_t current = core->current_inst;
  register mqword_t reg1 = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  core->registers[reg1] =
      ((float)core->registers[reg1] + (float)core->registers[reg2]);
  merry_cmp_floats32(core, core->registers[reg1], core->registers[reg2]);
}

_exec_(fsub32) {
  register mqword_t current = core->current_inst;
  register mqword_t reg1 = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  core->registers[reg1] =
      ((float)core->registers[reg1] - (float)core->registers[reg2]);
  merry_cmp_floats32(core, core->registers[reg1], core->registers[reg2]);
}

_exec_(fmul32) {
  register mqword_t current = core->current_inst;
  register mqword_t reg1 = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  core->registers[reg1] =
      ((float)core->registers[reg1] * (float)core->registers[reg2]);
  merry_cmp_floats32(core, core->registers[reg1], core->registers[reg2]);
}

_exec_(fdiv32) {
  register mqword_t current = core->current_inst;
  register mqword_t reg1 = (current >> 4) & 15;
  register mqword_t reg2 = (current) & 15;
  // divide by zero should result in infinity
  core->registers[reg1] =
      ((float)core->registers[reg1] / (float)core->registers[reg2]);
  merry_cmp_floats32(core, core->registers[reg1], core->registers[reg2]);
}

_exec_(movesx_imm8) {
  register mqword_t current = core->current_inst;
  register mqword_t lbyte = current & 0xFF; // this is only 1 byte long
  if (lbyte >> 7 == 1)
    lbyte = _sign_extend8_(lbyte);
  core->registers[(current >> 48) & 15] = lbyte;
}

_exec_(movesx_imm16) {
  register mqword_t current = core->current_inst;
  register mqword_t lbyte = current & 0xFFFF;
  if (lbyte >> 15 == 1)
    lbyte = _sign_extend16_(lbyte);
  core->registers[(current >> 48) & 15] = lbyte;
}

_exec_(movesx_imm32) {
  register mqword_t current = core->current_inst;
  register mqword_t lbyte = current & 0xFFFFFF;
  if (lbyte >> 31 == 1)
    lbyte = _sign_extend32_(lbyte);
  core->registers[(current >> 48) & 15] = lbyte;
}

_exec_(movesx_reg8) {
  register mqword_t current = core->current_inst;
  register mqword_t lbyte = core->registers[(current) & 15] & 0xFF;
  if (lbyte >> 7 == 1)
    lbyte = _sign_extend8_(lbyte);
  core->registers[(current >> 4) & 15] = lbyte;
}

_exec_(movesx_reg16) {
  register mqword_t current = core->current_inst;
  register mqword_t lbyte = core->registers[(current) & 15] & 0xFFFF;
  if (lbyte >> 15 == 1)
    lbyte = _sign_extend16_(lbyte);
  core->registers[(current >> 4) & 15] = lbyte;
}

_exec_(movesx_reg32) {
  register mqword_t current = core->current_inst;
  register mqword_t lbyte = core->registers[(current) & 15] & 0xFFFFFF;
  if (lbyte >> 31 == 1)
    lbyte = _sign_extend32_(lbyte);
  core->registers[(current >> 4) & 15] = lbyte;
}

// we don't need zero extended version but we do need the sign extended version
// of MOV

_exec_(call) {
  // we need to save the current state of the SP and BP
  // the parameters for the call should be already pushed
  // the parameters can then be accessed using the sva instruction

  /// NOTE: It is to be made sure that the parameters and values of other
  /// procedures should not be meddled with
  if (_is_stack_full_(core) || !_check_stack_lim_(core, 5)) {
    // the stack is full and we cannot perform this call
    // the stack must at least have 5 addresses free to be able to perform a
    // call
    merry_requestHdlr_panic(MERRY_STACK_OVERFLOW, core->core_id); // panic
    core->stop_running = mtrue;
    return;
  }
  (core->sp++);
  core->stack_mem[(core->sp)] = core->bp; // save the BP
  core->bp = core->sp;
}

_exec_(ret) {
  // Restore everything to its older state
  // also check if the stack is empty
  if (_is_stack_empty_(core) || !_stack_has_atleast_(core, 1)) {
    merry_requestHdlr_panic(MERRY_STACK_UNDERFLOW, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->bp = core->stack_mem[core->bp]; // restore the BP
                                        // it is the program's job to restore SP
                                        // to its desired position
  core->sp--;
}

_exec_(sva) {
  // op1 is the destination register and op2 is the offset value
  // first check if the offset value is valid
  register mqword_t current = core->current_inst;
  register mqword_t off = current & 0xFFFF;
  if (core->bp < off) {
    merry_requestHdlr_panic(MERRY_INVALID_VARIABLE_ACCESS, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  // now we can put the value from the stack
  /// NOTE: We are not popping the values and the original values cannot be
  /// changed here NOTE: Even the value of BP can be taken but in case of svc,
  /// the value of BP can be changed which is undesirable
  core->registers[(current >> 48) & 15] = core->stack_mem[core->bp - off];
}

_exec_(gss_imm) {
  register mqword_t current = core->current_inst;
  register mqword_t off = current & 0xFFFF;
  if ((core->bp + off) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_) {
    merry_requestHdlr_panic(MERRY_INVALID_VARIABLE_ACCESS, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[(current >> 48) & 15] = core->stack_mem[core->bp + off];
}

_exec_(svc) {
  // the same as sva
  register mqword_t current = core->current_inst;
  register mqword_t off = current & 0xFFFF;
  if (core->bp < off || off == 1) {
    merry_requestHdlr_panic(MERRY_INVALID_VARIABLE_ACCESS, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->stack_mem[core->bp - off] = core->registers[(current >> 48) & 15];
}

_exec_(sss_imm) {
  // the same as gss
  register mqword_t current = core->current_inst;
  register mqword_t off = current & 0xFFFF;
  if ((core->bp + off) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_) {
    merry_requestHdlr_panic(MERRY_INVALID_VARIABLE_ACCESS, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->stack_mem[core->bp + off] = core->registers[(current >> 48) & 15];
}

_exec_(sva_mem) {
  register mqword_t current = core->current_inst;
  mqword_t off;
  if (merry_dmemory_read_word(core->data_mem, current & 0xFFFFFFFFFFFF, &off) ==
      RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  if (core->bp < off) {
    merry_requestHdlr_panic(MERRY_INVALID_VARIABLE_ACCESS, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->registers[(current >> 48) & 15] = core->stack_mem[core->bp - off];
}

_exec_(svc_mem) {
  register mqword_t current = core->current_inst;
  mqword_t off;
  if (merry_dmemory_read_word(core->data_mem, current & 0xFFFFFFFFFFFF, &off) ==
      RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // make sure that svc doesn't affect the old value of BP
  if (core->bp < off || (off == 1)) {
    merry_requestHdlr_panic(MERRY_INVALID_VARIABLE_ACCESS, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  core->stack_mem[core->bp - off] = core->registers[(current >> 48) & 15];
}

_exec_(push_imm) {
  // push the provided immediate value onto the stack
  // it is assumed that the value is unsigned and hence zero extended
  if (_is_stack_full_(core)) {
    merry_requestHdlr_panic(MERRY_STACK_OVERFLOW, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  core->sp++;
  core->stack_mem[core->sp] = merry_core_get_immediate(core);
}

_exec_(push_reg) {
  // same as above
  if (_is_stack_full_(core)) {
    merry_requestHdlr_panic(MERRY_STACK_OVERFLOW, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  core->sp++;
  core->stack_mem[core->sp] = core->registers[core->current_inst & 15];
}

_exec_(pop) {
  // the oppsite of above
  if (_is_stack_empty_(core)) {
    merry_requestHdlr_panic(MERRY_STACK_UNDERFLOW, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  core->registers[core->current_inst & 15] = core->stack_mem[core->sp];
  core->sp--;
}

_exec_(pusha) {
  if (!_check_stack_lim_(core, REGR_COUNT)) {
    // the stack cannot hold all the register values
    merry_requestHdlr_panic(MERRY_STACK_OVERFLOW, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  for (msize_t i = 0; i < REGR_COUNT; i++) {
    // now move one by one
    core->sp++;
    core->stack_mem[core->sp] = core->registers[i];
  }
}

_exec_(popa) {
  if (!_stack_has_atleast_(core, REGR_COUNT)) {
    // the stack doesn't have enough values
    merry_requestHdlr_panic(MERRY_STACK_UNDERFLOW, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  for (int i = 15; i >= 0; i--) {
    // now move one by one
    core->registers[i] = core->stack_mem[core->sp];
    core->sp--;
  }
}

_lexec_(load, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_qword(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(store, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_qword(
          core->data_mem, address,
          core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_lexec_(loadw, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_word(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(storew, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_word(
          core->data_mem, address,
          core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_lexec_(loadd, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_dword(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(stored, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_dword(
          core->data_mem, address,
          core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_lexec_(loadb, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_byte(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(storeb, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_byte(
          core->data_mem, address,
          core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_lexec_(load_reg, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_qword(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(store_reg, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_qword(
          core->data_mem, address,
          core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_lexec_(loadw_reg, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_word(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(storew_reg, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_word(
          core->data_mem, address,
          core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_lexec_(loadd_reg, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_dword(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(stored_reg, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_dword(
          core->data_mem, address,
          core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_lexec_(loadb_reg, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_byte(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(storeb_reg, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_byte(
          core->data_mem, address,
          core->registers[(core->current_inst >> 4) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_exec_(excg8) {
  // just exchange the bytes and don't overwrite anything
  register mqword_t curr = core->current_inst;
  register mqword_t reg1 = core->registers[(curr >> 4) & 15];
  register mqword_t reg2 = core->registers[curr & 15];
  core->registers[(curr >> 4) & 15] &= (0xFFFFFFFFFFFFFF00 | (reg2 & 0xFF));
  core->registers[curr & 15] &= (0xFFFFFFFFFFFFFF00 | (reg1 & 0xFF));
}

_exec_(excg16) {
  // just exchange the bytes and don't overwrite anything
  register mqword_t curr = core->current_inst;
  register mqword_t reg1 = core->registers[(curr >> 4) & 15];
  register mqword_t reg2 = core->registers[curr & 15];
  core->registers[(curr >> 4) & 15] &= (0xFFFFFFFFFFFFFF00 | (reg2 & 0xFFFF));
  core->registers[curr & 15] &= (0xFFFFFFFFFFFFFF00 | (reg1 & 0xFFFF));
}

_exec_(excg32) {
  // just exchange the bytes and don't overwrite anything
  register mqword_t curr = core->current_inst;
  register mqword_t reg1 = core->registers[(curr >> 4) & 15];
  register mqword_t reg2 = core->registers[curr & 15];
  core->registers[(curr >> 4) & 15] &= (0xFFFFFFFFFFFFFF00 | (reg2 & 0xFFFFFF));
  core->registers[curr & 15] &= (0xFFFFFFFFFFFFFF00 | (reg1 & 0xFFFFFF));
}

_exec_(excg) {
  // just exchange the bytes and don't overwrite anything
  register mqword_t curr = core->current_inst;
  register mqword_t reg1 = core->registers[(curr >> 4) & 15];
  register mqword_t reg2 = core->registers[curr & 15];
  core->registers[(curr >> 4) & 15] = reg2;
  core->registers[curr & 15] = reg1;
}

_lexec_(atm_load, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_qword_atm(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(atm_store, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_qword_atm(
          core->data_mem, address,
          core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_lexec_(atm_loadw, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_word_atm(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(atm_storew, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_word_atm(
          core->data_mem, address,
          core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_lexec_(atm_loadd, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_dword_atm(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(atm_stored, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_dword_atm(
          core->data_mem, address,
          core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

_lexec_(atm_loadb, mqword_t address) {
  // read from the given address
  if (merry_dmemory_read_byte_atm(
          core->data_mem, address,
          &core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be loaded
}

_lexec_(atm_storeb, mqword_t address) {
  // store to the given address from the given register
  if (merry_dmemory_write_byte_atm(
          core->data_mem, address,
          core->registers[(core->current_inst >> 48) & 15]) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  // the value should be stored
}

// not used anymore
_exec_(syscall) {
  /*
    The convention for this is that the program should provide the arguments in
    order: Ma, M1, M2, M3, M4, M4, M5 The order of the arguments can be looked
    up in the ABI for the platform. Ma is the syscall number
  */
  /**
   * It has come to light that directly using syscall isn't the brightest of the
   * options. Firstly, the program gets too much power. A program has the
   * potential to overwrite the VM and take over the current VM process. The
   * program can also create new processes which would be a disaster. The second
   * problem lies with the fact that we have to take care of many things during
   * the actual syscall such that the arguments are passed correctly. The second
   * problem is handleable in the library but the first problem isn't. It should
   * be in the interest of people to not do things that would be bad to the
   * program but some cannot be trusted but there is nothing we can do except
   * for including checks for certain syscalls. The checks would be
   * comprehensive and long causing the syscalls to take a long time to execute.
   * Instead of that, for now atleast, the only thing that can be done is hope
   * everyone doesn't try to exploit this.
   */
#ifdef _USE_LINUX_
  core->registers[Ma] =
      syscall(core->registers[Ma], core->registers[M1], core->registers[M2],
              core->registers[M3], core->registers[M4], core->registers[M5]);
  core->registers[Mb] = merry_get_errno();
#endif
}

_lexec_(push_mem, mem_read func) {
  register mqword_t address = core->current_inst & 0xFFFFFFFFFFFF;
  mqword_t temp = 0;
  if (func(core->data_mem, address, &temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
  if (_is_stack_full_(core)) {
    merry_requestHdlr_panic(MERRY_STACK_OVERFLOW, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  core->sp++;
  core->stack_mem[core->sp] = temp;
}

_lexec_(pop_mem, mem_write func) {
  register mqword_t address = core->current_inst & 0xFFFFFFFFFFFF;
  if (_is_stack_empty_(core)) {
    merry_requestHdlr_panic(MERRY_STACK_UNDERFLOW, core->core_id);
    core->stop_running = mtrue;
    return; // failure
  }
  register mqword_t temp = core->stack_mem[core->sp];
  core->sp--;
  if (func(core->data_mem, address, temp) == RET_FAILURE) {
    merry_requestHdlr_panic(core->data_mem->error, core->core_id);
    core->stop_running = mtrue;
    return;
  }
}
