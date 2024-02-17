#ifndef _MERRY_IMP_
#define _MERRY_IMP_

#include "../../../utils/merry_types.h"

struct MerryFlagRegister;

extern void _update_flags_(struct MerryFlagRegister *flag);

extern mqword_t _inc_inst_(mqword_t _to_inc);
extern mqword_t _dec_inst_(mqword_t _to_dec);

extern void _cmp_inst_(mqword_t val1, mqword_t val2, struct MerryFlagRegister *reg);

#endif