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

#endif