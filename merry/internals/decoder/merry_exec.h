#ifndef _MERRY_EXEC_
#define _MERRY_EXEC_

#include "../../../utils/merry_logger.h"
#include "../merry_request.h"
#include "../imp/merry_arithmetic.h"

struct MerryCore;

// The function structure
#define _exec_(name) void merry_execute_##name(struct MerryCore *core)

#define _sign_extend8_(val) val | 0xFFFFFFFFFFFFFF00
#define _sign_extend16_(val) val | 0xFFFFFFFFFFFF0000
#define _sign_extend32_(val) val | 0xFFFFFFFFFF000000

// execute the halt instruction
_exec_(halt);

// arithmetic instructions
_exec_(add_imm);
_exec_(add_reg);
_exec_(sub_imm);
_exec_(sub_reg);
_exec_(mul_imm);
_exec_(mul_reg);
_exec_(div_imm);
_exec_(div_reg);
_exec_(mod_imm);
_exec_(mod_reg);

_exec_(iadd_imm);
_exec_(iadd_reg);
_exec_(isub_imm);
_exec_(isub_reg);
_exec_(imul_imm);
_exec_(imul_reg);
_exec_(idiv_imm);
_exec_(idiv_reg);
_exec_(imod_imm);
_exec_(imod_reg);

// move instructions
_exec_(move_imm);
_exec_(move_reg);
_exec_(move_reg8);
_exec_(move_reg16);
_exec_(move_reg32);

_exec_(movesx_reg8);
_exec_(movesx_reg16);
_exec_(movesx_reg32);
_exec_(movesx_imm8);
_exec_(movesx_imm16);
_exec_(movesx_imm32);

#endif