#ifndef _MERRY_EXEC_
#define _MERRY_EXEC_

#include "../../../utils/merry_logger.h"
#include "../merry_request.h"

struct MerryCore;

// The function structure
#define _exec_(name) void merry_execute_##name(struct MerryCore *core)

// execute the halt instruction
_exec_(halt);

#endif