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

#include "../../utils/merry_config.h"
#include "../../utils/merry_types.h"
#include "../lib/include/merry_memory_allocator.h"
#include "../../sys/merry_thread.h"
#include "merry_internals.h"
#include "../../sys/merry_mem.h" // for allocating a page for the stack that the core manages
#include "merry_memory.h"
#include "../includes/merry_errors.h"

typedef struct MerryCore MerryCore;
// typedef union MerryRegister MerryRegister;
typedef struct MerryFlagRegister MerryFlagRegister;

/*
 The behaviour of Unions is very different based on different architectures, endianness and the whim of the compiler as well.
 This could have been very safe, fast and useful as well as saved a ton of time but unfortunately, it is not that predictable.
*/
// union MerryRegister
// {
//     mqword_t _register_;
//     unsigned int _lhalf;
// };

struct MerryFlagRegister
{
    unsigned long zero : 1;
    unsigned long equal : 1;
    unsigned long greater : 1;
    unsigned long overflow : 1; // CARRY flag for signed arithmetic
    unsigned long carry : 1;    // CARRY flag for unsigned arithmetic
    unsigned long negative : 1;
    unsigned long res : 58; // reserved
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
    MerryMemory *data_mem; // the data memory
    MerryMemory *inst_mem; // the instruction memory
    // Each address of the stack stores 8 bytes which implies each push or pop pushes and pops 8 bytes
    // there are no need for alignments
    mqptr_t stack_mem;       // the private stack of the core
    mqptr_t registers;       // the core's registers
    mqword_t sp, bp, pc, ir; // four registers that is inaccessible to anything and are changeable indirectly
    mqword_t core_id;        // this register holds the id provided to it which is unique
    // some important flags
    // mbool_t should_wait;  // tell the core to wait until signaled[MAY NOT BE NEEDED]
    mbool_t stop_running; // tell the core to stop executing and shut down
    // to get maximum performance, we want to use everything we can
    // When a core is executing instructions and it is certain that the values and pages it accesses are not accessed by
    // other cores, it can set this flag and access memory pages without mutex locks which is faster.
    // If this flag is set but other cores access this core's pages and values then it is not known what behaviour might happen
    mbool_t _is_private;
};

// initialize a new core
MerryCore *merry_core_init(MerryMemory *inst_mem, MerryMemory *data_mem, msize_t id);

void merry_core_destroy(MerryCore *core);

mptr_t merry_runCore(mptr_t core);

#endif