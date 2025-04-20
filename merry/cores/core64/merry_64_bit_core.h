#ifndef _MERRY_64_BIT_CORE_
#define _MERRY_64_BIT_CORE_

#include "merry_core_base.h"
#include "merry_utils.h"
#include "merry_types.h"
#include "merry_platform.h"
#include "merry_ram.h"
#include "merry_state.h"
#include "merry_dynamic_list.h"
#include "merry_core_types.h"
#include "merry_64_bit_core_isa.h"
#include "merry_graves_request_queue.h"
#include <stdlib.h>

#define _MERRY_64_BIT_CORE_REGR_COUNT_ 16

enum
{
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
  R13,
  R14,
  ACC,
  REG_COUNT_64,
};

typedef struct Merry64BitCore Merry64BitCore;

struct Merry64BitCore
{
  MerryCoreBase *base;
  mqword_t regr[REG_COUNT_64];
  MerryFlagsRegr fregr;
  mqword_t pc; // program counter
  mqword_t sp, bp;
  MerryRAM *stack;
  MerryRAM *ram;
  MerryRAM* iram; // only for instructions
  MerryDynamicList* tbs; // Trace-back Stack
  MerryGravesRequest *req;
};

MerryCoreBase *merry_64_bit_core_base(MerryState *state);

void *merry_64_bit_core_init(MerryCoreBase* base, MerryRAM *ram, MerryRAM* iram, maddress_t start_point);

void merry_64_bit_core_destroy(void *cptr);

_THRET_T_ merry_64_bit_core_run(void *arg);

#endif
