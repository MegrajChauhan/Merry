#ifndef _MERRY_IMP_
#define _MERRY_IMP_

#include "../../../utils/merry_types.h"

struct MerryFlagRegister;

extern void _update_flags_(struct MerryFlagRegister *flag);
extern mqword_t _add_inst_(mqword_t op1, mqword_t op2);
extern mqword_t _sub_inst_(mqword_t op1, mqword_t op2);
extern mqword_t _mul_inst_(mqword_t op1, mqword_t op2);
extern mqword_t _div_inst_(mqword_t op1, mqword_t op2);
extern mqword_t _mod_inst_(mqword_t op1, mqword_t op2);

extern mqword_t _iadd_inst_(mqword_t op1, mqword_t op2);
extern mqword_t _isub_inst_(mqword_t op1, mqword_t op2);
extern mqword_t _imul_inst_(mqword_t op1, mqword_t op2);
extern mqword_t _idiv_inst_(mqword_t op1, mqword_t op2);
extern mqword_t _imod_inst_(mqword_t op1, mqword_t op2);

extern mqword_t _inc_inst_(mqword_t _to_inc);
extern mqword_t _dec_inst_(mqword_t _to_dec);

void _cmp_inst_(mqword_t val1, mqword_t val2, struct MerryFlagRegister *reg);

#endif