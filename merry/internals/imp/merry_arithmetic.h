#ifndef _MERRY_ARITHMETIC_
#define _MERRY_ARITHMETIC_

#include "../../../utils/merry_types.h"

struct MerryFlagRegister;

extern void _update_flags_(struct MerryFlagRegister *flag);
extern mqword_t _add_inst_(mqword_t op1, mqword_t op2);

#endif