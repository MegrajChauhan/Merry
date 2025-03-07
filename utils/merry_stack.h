#ifndef _MERRY_STACK_
#define _MERRY_STACK_

#include <merry_utils.h>
#include <merry_types.h>
#include <stdlib.h>

// The stacks are going to be static only
// The stack also stores only pointers(or type-casted integers)
typedef struct MerryStack MerryStack;

struct MerryStack
{
    mptr_t *buf;
    msize_t sp;
    msize_t cap;
    msize_t sp_max; // the maximum value SP can take
};

#define merry_is_stack_full(stack) ((stack)->sp >= (stack)->sp_max)

MerryStack *merry_stack_init(msize_t cap);

mret_t merry_stack_push(MerryStack *stack, mptr_t value);

mptr_t merry_stack_pop(MerryStack *stack);

void merry_stack_destroy(MerryStack *stack);

#endif