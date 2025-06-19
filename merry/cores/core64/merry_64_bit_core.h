#ifndef _MERRY_64_BIT_CORE_
#define _MERRY_64_BIT_CORE_

#include "merry_64_bit_core_isa.h"
#include "merry_core_base.h"
#include "merry_core_types.h"
#include "merry_graves_request_queue.h"
#include "merry_platform.h"
#include "merry_ram.h"
#include "merry_stack.h"
#include "merry_state.h"
#include "merry_types.h"
#include "merry_utils.h"
#include <merry_core_base_mem_access.h>
#include <stdlib.h>

#define _MERRY_64_BIT_CORE_REGR_COUNT_ 16
#define EXEC64(name)                                                           \
  void name##64(Merry64BitCore * core, MerryHostMemLayout inst)
#define _MERRY_64_BIT_CORE_TBS_LIM_ 50

enum {
  ACC,
  BP,
  SP,
  R0,
  R1,
  R2,
  R3,
  R4,
  R5,
  R6,
  R7,
  R8,
  R9,
  R10,
  R11,
  R12,
  GPC_64_LAST_REGR = 15,
  REG_COUNT_GPC_64,
};

typedef struct Merry64BitCore Merry64BitCore;
typedef struct Merry64BitCoreState Merry64BitCoreState;

struct Merry64BitCore {
  MerryCoreBase *base;
  mqword_t regr[REG_COUNT_GPC_64];
  MerryFlagsRegr fregr;
  MerryFFlagsRegr ffregr;
  mqword_t pc; // program counter
  MerryRAM *stack;
  MerryStack *tbs;    // Trace-back Stack
  MerryStack *cstack; // Call Stack
  MerryGravesRequest *req;
};

struct Merry64BitCoreState {
  mqword_t regr[REG_COUNT_GPC_64];
  MerryFlagsRegr fregr;
  MerryFFlagsRegr ffregr;
  mqword_t pc;
  mbool_t state_valid;
};

MerryCoreBase *merry_64_bit_core_base(MerryState *state);

void *merry_64_bit_core_init(MerryCoreBase *base, MerryRAM *ram, MerryRAM *iram,
                             maddress_t start_point);

void merry_64_bit_core_destroy(void *cptr);

msize_t merry_64_bit_core_find_free_state(Merry64BitCore *core);

mret_t merry_64_bit_core_save_state(void *cptr);

mret_t merry_64_bit_core_replace_state(void *cptr, msize_t id);

mret_t merry_64_bit_core_del_state(void *cptr, msize_t id);

mret_t merry_64_bit_core_jmp_state(void *cptr, msize_t id);

_THRET_T_ merry_64_bit_core_run(void *arg);

EXEC64(add_imm);
EXEC64(add_reg);
EXEC64(sub_imm);
EXEC64(sub_reg);
EXEC64(mul_imm);
EXEC64(mul_reg);
EXEC64(div_imm);
EXEC64(div_reg);
EXEC64(mod_imm);
EXEC64(mod_reg);

EXEC64(iadd_imm);
EXEC64(iadd_reg);
EXEC64(isub_imm);
EXEC64(isub_reg);
EXEC64(imul_imm);
EXEC64(imul_reg);
EXEC64(idiv_imm);
EXEC64(idiv_reg);
EXEC64(imod_imm);
EXEC64(imod_reg);

EXEC64(fadd);
EXEC64(fsub);
EXEC64(fmul);
EXEC64(fdiv);

EXEC64(fadd32);
EXEC64(fsub32);
EXEC64(fmul32);
EXEC64(fdiv32);

EXEC64(add_memb);
EXEC64(add_memw);
EXEC64(add_memd);
EXEC64(add_memq);

EXEC64(sub_memb);
EXEC64(sub_memw);
EXEC64(sub_memd);
EXEC64(sub_memq);

EXEC64(mul_memb);
EXEC64(mul_memw);
EXEC64(mul_memd);
EXEC64(mul_memq);

EXEC64(div_memb);
EXEC64(div_memw);
EXEC64(div_memd);
EXEC64(div_memq);

EXEC64(mod_memb);
EXEC64(mod_memw);
EXEC64(mod_memd);
EXEC64(mod_memq);

EXEC64(fadd64_mem);
EXEC64(fsub64_mem);
EXEC64(fmul64_mem);
EXEC64(fdiv64_mem);

EXEC64(fadd32_mem);
EXEC64(fsub32_mem);
EXEC64(fmul32_mem);
EXEC64(fdiv32_mem);

EXEC64(mov_imm);
EXEC64(movf32);

// mov_reg, mov_reg8, mov_reg16, mov_reg32 don't need a dedicated function

EXEC64(movesx_imm8);
EXEC64(movesx_imm16);
EXEC64(movesx_imm32);
EXEC64(movesx_reg8);
EXEC64(movesx_reg16);
EXEC64(movesx_reg32);

EXEC64(excg);
EXEC64(excg8);
EXEC64(excg16);
EXEC64(excg32);

// mov8, mov16, mov32 also won't need dedicated functions
// conditional moves also won't require dedicated functions

EXEC64(call);
EXEC64(ret);
EXEC64(call_reg);

EXEC64(push_immb);
EXEC64(push_immw);
EXEC64(push_immd);
EXEC64(push_immq);
EXEC64(push_reg);
EXEC64(popb);
EXEC64(popw);
EXEC64(popd);
EXEC64(popq);
EXEC64(pusha);
EXEC64(popa);

EXEC64(push_memb);
EXEC64(push_memw);
EXEC64(push_memd);
EXEC64(push_memq);
EXEC64(pop_memb);
EXEC64(pop_memw);
EXEC64(pop_memd);
EXEC64(pop_memq);

EXEC64(loadsb);
EXEC64(loadsw);
EXEC64(loadsd);
EXEC64(loadsq);

EXEC64(storesb);
EXEC64(storesw);
EXEC64(storesd);
EXEC64(storesq);

EXEC64(and_imm);
EXEC64(or_imm);
EXEC64(xor_imm);

EXEC64(cmp_imm);
EXEC64(cmp_imm_memb);
EXEC64(cmp_imm_memw);
EXEC64(cmp_imm_memd);
EXEC64(cmp_imm_memq);

EXEC64(sin);
EXEC64(sout);

EXEC64(loadb);
EXEC64(storeb);
EXEC64(loadw);
EXEC64(storew);
EXEC64(loadd);
EXEC64(stored);
EXEC64(loadq);
EXEC64(storeq);

EXEC64(loadb_reg);
EXEC64(storeb_reg);
EXEC64(loadw_reg);
EXEC64(storew_reg);
EXEC64(loadd_reg);
EXEC64(stored_reg);
EXEC64(loadq_reg);
EXEC64(storeq_reg);

EXEC64(atm_loadb);
EXEC64(atm_loadw);
EXEC64(atm_loadd);
EXEC64(atm_loadq);

EXEC64(atm_storeb);
EXEC64(atm_storew);
EXEC64(atm_stored);
EXEC64(atm_storeq);

EXEC64(atm_loadb_reg);
EXEC64(atm_loadw_reg);
EXEC64(atm_loadd_reg);
EXEC64(atm_loadq_reg);

EXEC64(atm_storeb_reg);
EXEC64(atm_storew_reg);
EXEC64(atm_stored_reg);
EXEC64(atm_storeq_reg);

EXEC64(cmpxchg);
EXEC64(cmpxchg_reg);

EXEC64(whdlr);

#endif
