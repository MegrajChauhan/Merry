#ifndef _MERRY_STACK_
#define _MERRY_STACK_

#include "merry_types.h"
#include <stdlib.h>

typedef struct MerryStack MerryStack;

struct MerryStack
{
    mqptr_t array;          // the stack array
    msize_t size;           // the stack size
    msize_t sp;             // the current position of the stack
    mbool_t dynamic;        // should the stack resize when full?
    msize_t upper_lim;      // set an upper limit to resizing
    msize_t add_per_resize; // how many addresses to add per resize
    /*
      Whereever sp points, a value is pushed there and then only it is incremented.
      The opposite happens for popping
    */
};

#define stack_full(stack) (stack->sp >= stack->size)
#define stack_empty(stack) (stack->sp == 0)

MerryStack *merry_init_stack(msize_t len, mbool_t dynamic, msize_t upper_lim, msize_t per_resize);

void merry_destroy_stack(MerryStack *stack);

mret_t merry_stack_push(MerryStack *st, mqword_t _to_push);

mret_t merry_stack_pop(MerryStack *st, mqptr_t store_in);

#endif