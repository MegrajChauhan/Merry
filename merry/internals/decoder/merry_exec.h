#ifndef _MERRY_EXEC_
#define _MERRY_EXEC_

#include "../../../utils/merry_logger.h"
#include "../merry_request.h"
#include "../imp/merry_arithmetic.h"

struct MerryCore;

// The function structure
#define _exec_(name) void merry_execute_##name(struct MerryCore *core)

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

_exec_(iadd_imm);
_exec_(iadd_reg);
_exec_(isub_imm);
_exec_(isub_reg);
_exec_(imul_imm);
_exec_(imul_reg);
_exec_(idiv_imm);
_exec_(idiv_reg);

#endif