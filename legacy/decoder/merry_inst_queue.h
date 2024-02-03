#ifndef _MERRY_INST_QUEUE_
#define _MERRY_INST_QUEUE_

#include <stdlib.h>
#include "../../lib/include/merry_queue.h"
#include "merry_opcodes.h"
#include "../merry_inst.h"
#include "merry_exec.h"
#include "../../../utils/merry_stack.h"

typedef struct MerryInstQueue MerryInstQueue;
typedef struct MerryInstQueueNode MerryInstQueueNode;
typedef struct MerryInstruction MerryInstruction;

// A function that executes the instruction should take a MerryInstruction *, MerryCore *
// Any error generated during this  will be reported by the executing function itself

_MERRY_CREATE_QUEUE_NODE_NOPTR_(MerryInstruction, MerryInstQueueNode)
_MERRY_CREATE_QUEUE_NOPTR_(MerryInstQueue, MerryInstQueueNode)

static MerryStack *d_stack;

MerryInstQueue *merry_inst_queue_init(msize_t queue_len);

mbool_t merry_inst_queue_push_instruction(MerryInstQueue *queue, MerryInstruction inst);

mbool_t merry_inst_queue_pop_instruction(MerryInstQueue *queue, MerryInstruction *inst);

void merry_inst_queue_destroy(MerryInstQueue *queue);

void merry_inst_queue_hazard(MerryInstQueue *queue, MerryInstruction *inst);

void merry_inst_queue_set_stack(MerryStack *stack);

MerryInstruction *merry_inst_queue_get_next_tail(MerryInstQueue *queue);

#endif