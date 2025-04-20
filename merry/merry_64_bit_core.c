#include "merry_64_bit_core.h"

MerryCoreBase *merry_64_bit_core_base(MerryState *state) {
  MerryCoreBase *base = (MerryCoreBase *)malloc(sizeof(MerryCoreBase));
  if (!base) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    merry_provide_context(*state, _MERRY_CORE_BASE_INITIALIZATION_);
    return RET_NULL;
  }

  base->init_func = merry_64_bit_core_init;
  base->free_func = merry_64_bit_core_destroy;
  base->exec_func = merry_64_bit_core_run;
  base->bus_len = 8;
  base->pause = 0;

  if (merry_cond_init(&base->cond) == RET_FAILURE) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OBTAIN_COND_);
    merry_provide_context(*state, _MERRY_CORE_BASE_INITIALIZATION_);
    free(base);
    return RET_NULL;
  }

  if (merry_mutex_init(&base->lock) == RET_FAILURE) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OBTAIN_LOCK_);
    merry_provide_context(*state, _MERRY_CORE_BASE_INITIALIZATION_);
    merry_cond_destroy(base->cond);
    free(base);
    return RET_NULL;
  }

  base->core_type = __CORE_64_BIT;
  merry_assign_state(base->state, _MERRY_ORIGIN_NONE_, 0);
  base->state.child_state = NULL;
  return base;
}

void *merry_64_bit_core_init(MerryCoreBase *base, MerryRAM *ram, MerryRAM *iram,
                             maddress_t start_point) {
  merry_check_ptr(base);
  merry_check_ptr(ram);

  Merry64BitCore *core = (Merry64BitCore *)malloc(sizeof(Merry64BitCore));
  if (!core) {
    merry_assign_state(base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    merry_provide_context(base->state, _MERRY_CORE_INITIALIZATION_);
    return RET_NULL;
  }
  core->base = base;
  core->pc = start_point;
  core->pc = start_point;
  core->ram = ram;
  core->iram = iram;
  core->sp = 0;
  core->bp = 0;

  MerryRAM *stack = merry_create_RAM(2, &base->state); // 2MB

  if (!stack) {
    merry_provide_context(base->state, _MERRY_CORE_INITIALIZATION_);
    free(core);
    return RET_NULL;
  }

  MerryDynamicList *list = merry_create_dynamic_list(50, 8, &base->state);

  if (!list) {
    merry_provide_context(base->state, _MERRY_CORE_INITIALIZATION_);
    merry_destroy_RAM(stack);
    free(core);
    return RET_NULL;
  }

  if ((core->req = (MerryGravesRequest *)malloc(sizeof(MerryGravesRequest))) ==
      NULL) {
    merry_assign_state(base->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    merry_provide_context(base->state, _MERRY_CORE_INITIALIZATION_);
    merry_destroy_RAM(stack);
    merry_destroy_dynamic_list(list);
    free(core);
    return RET_NULL;
  }

  core->req->base = core->base;
  core->stack = stack;
  core->tbs = list;

  return (void *)core;
}

void merry_64_bit_core_destroy(void *cptr) {
  merry_check_ptr(cptr);
  Merry64BitCore *c = (Merry64BitCore *)cptr;

  merry_mutex_destroy(c->base->lock);
  merry_cond_destroy(c->base->cond);

  free(c->req);
  free(c->base);
  merry_destroy_RAM(c->stack);
  merry_destroy_dynamic_list(c->tbs);

  // The RAM is handled by Graves

  free(cptr);
}

_THRET_T_ merry_64_bit_core_run(void *arg) {
  merry_check_ptr(arg);
  register Merry64BitCore *core = (Merry64BitCore *)arg;
  MerryHostMemLayout current;
  register mqword_t PC = core->pc;

  while (mtrue) {
    switch (core->base->pause) {
    case 1:
      merry_cond_wait(core->base->cond, core->base->lock);
      break;
    default:
      if (merry_RAM_read_qword(core->iram, PC, &current.whole_word,
                               &core->base->state) == RET_FAILURE) {
        // Tell Graves about the error[Pause execution]
        merry_SEND_REQUEST(core->req);
      }

      // We will use the old structure from Merry for 64-bit core
      switch (current.bytes.b0) {
      case __NOP64:
        break;
      case __ADD64_REG:
        core->regr[current.bytes.b6 & REG_COUNT_64] +=
            core->regr[current.bytes.b7 & REG_COUNT_64];
        break;
      }
      PC++;
      merry_update_flags_regr(&core->fregr);
    }
  }

  return (_THRET_T_)(&core->regr[R0]);
}
