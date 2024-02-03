/*
 * The Virtual executing core of the Merry VM
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
#ifndef _MERRY_CORE_
#define _MERRY_CORE_

#include <stdlib.h>
#include "merry_internals.h"
#include "merry_memory.h"

typedef struct MerryCore MerryCore;
// typedef union MerryRegister MerryRegister;
typedef struct MerryFlagRegister MerryFlagRegister;

// #include "merry_exec.h"
#include "decoder/merry_decode.h"

/*
 The behaviour of Unions is very different based on different architectures, endianness and the whim of the compiler as well.
 This could have been very safe, fast and useful as well as saved a ton of time but unfortunately, it is not that predictable.
*/
// union MerryRegister
// {
//     mqword_t _register_;
//     unsigned int _lhalf;
// };

#define flags_res(x, size) unsigned long x : size

#define _is_stack_full_(core) (core->sp == _MERRY_MEMORY_QS_PER_PAGE_)
#define _check_stack_lim_(core, size) ((_MERRY_MEMORY_QS_PER_PAGE_ - core->sp) > size)
#define _is_stack_empty_(core) (core->sp == 0)
#define _stack_has_atleast_(core, atleast) (core->sp >= atleast)

struct MerryFlagRegister
{
#if defined(_MERRY_HOST_CPU_AMD_)
    // This defines the flags structure for the AMD64 processors
    unsigned long carry : 1;     /*0th bit is the CF in AMD64 EFlags*/
    flags_res(r1, 1);            /*1 bit reserved here*/
    unsigned long parity : 1;    /*2th bit is the PF*/
    flags_res(r2, 1);            /*1 bit reserved here*/
    unsigned long aux_carry : 1; /*4th bit Aux Carry flag[NOT REALLY NEEDED AS BCD INSTRUCTIONS ARE NOT SUPPORTED]*/
    flags_res(r3, 1);            /*1 bit reserved here*/
    unsigned long zero : 1;      /*6th bit ZF*/
    unsigned long negative : 1;  /*7th bit SF or NG*/
    flags_res(r4, 2);            /*2 bit reserved here*/
    unsigned long overflow : 1;  /*10th bit is the OF*/
    unsigned long direction : 1; /*11th bit is the DF[NOT REALLY USEFUL YET BUT MAYBE WHEN IMPLEMENTING STRING RELATED INSTRUCTIONS]*/
    flags_res(rem_32, 20);
    flags_res(top_32, 32);
#endif
};

enum
{
    Ma, // general purpose register
    Mb, // general purpose register
    Mc, // general purpose register
    Md, // general purpose register
    Me, // general purpose register
    Mf, // general purpose register

    M1, // extra register for anything
    M2, // extra register for anything
    M3, // extra register for anything
    M4, // extra register for anything
    M5, // extra register for anything

    // most likely to be used for floating point numbers
    Mm1, // Unknown register for now
    Mm2, // Unknown register for now
    Mm3, // Unknown register for now
    Mm4, // Unknown register for now
    Mm5,

    // 4 inaccessible registers, 16 registers for any use but this count may be increased in the future
    REGR_COUNT,
};

struct MerryCore
{
    // firstly every core shares some variables with other cores
    // they also need some private variables unique to only them
    MerryCond *cond;  // the core's private condition variable
    MerryMutex *lock; // the core's private mutex lock
    // the core's memory
    MerryMemory *data_mem;       // the data memory
    MerryMemory *inst_mem;       // the instruction memory
    MerryDecoder *decoder;       // the core's decoder
    MerryThread *decoder_thread; // the decoder's thread
    // Merry *os;
    // Each address of the stack stores 8 bytes which implies each push or pop pushes and pops 8 bytes
    // there are no need for alignments
    mqptr_t stack_mem;      // the private stack of the core
    mqptr_t registers;      // the core's registers
    mqword_t sp, bp, pc;    // four registers that is inaccessible to anything and are changeable indirectly
    mqword_t core_id;       // this register holds the id provided to it which is unique
    MerryFlagRegister flag; // the flags register[This is 64 bits in length. A pointer would be the same length and so there really is no need to declare it as a pointer]
    // some important flags
    // mbool_t should_wait;  // tell the core to wait until signaled[MAY NOT BE NEEDED]
    mbool_t stop_running; // tell the core to stop executing and shut down
    // to get maximum performance, we want to use everything we can
    // When a core is executing instructions and it is certain that the values and pages it accesses are not accessed by
    // other cores, it can set this flag and access memory pages without mutex locks which is faster.
    // If this flag is set but other cores access this core's pages and values then it is not known what behaviour might happen
    mbool_t _is_private;
    MerryInstruction ir; // the current instruction
};

_MERRY_ALWAYS_INLINE void merry_core_zero_out_reg(MerryCore *core)
{
    for (msize_t i = 0; i < REGR_COUNT; i++)
    {
        core->registers[i] = 0;
    }
}

// initialize a new core
MerryCore *merry_core_init(MerryMemory *inst_mem, MerryMemory *data_mem, msize_t id);

void merry_core_destroy(MerryCore *core);

mptr_t merry_runCore(mptr_t core);

#endif