/*
 * MIT License
 *
 * Copyright (c) 2024 MegrajChauhan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
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

void merry_stack_popn(MerryStack *st);

#endif