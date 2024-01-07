/*
 * The omnipotent being of the Merry VM
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
#ifndef _MERRY_OS_
#define _MERRY_OS_

// This module is the backbone of the VM and controls everything
#include "../../utils/merry_config.h"
#include "../../utils/merry_types.h"
#include "../lib/include/merry_memory_allocator.h"
#include "../../sys/merry_thread.h"
#include "../includes/merry_errors.h"
#include "merry_memory.h"
#include "merry_core.h"

typedef struct Merry Merry;

struct Merry
{
    MerryCore **cores;      // the vcores
    MerryMemPage *inst_mem; // the instruction memory that every vcore shares
    MerryMemPage *data_mem; // the data memory that every vcore shares
    MerryMutex *_lock;      // the Manager's lock
    MerryCond *_cond;       // the Manager's cond
    // MerryCond *shared_cond; // this condition is shared among all cores
    msize_t core_count;     // the number of vcores
    mbool_t stop;           // tell the manager to stop the VM and exit
};


//this is just the abstraction for basic read with no modifications[Not atomic] 
#define merry_manager_mem_read_inst(os, address, store_in) merry_memory_read(os->inst_mem, address, store_in)
#define merry_manager_mem_read_data(os, address, store_in) merry_memory_read(os->data_mem, address, store_in)

/*
 The Manager assigns core ids to every core it manages which then helps in identifying the cores.
 The potential use for shared_cond is that when performing atomic operations, we can block other vcores and unblock them at the same time as well.
 We may also use the shared_cond for things where we have to temporarily stop all the vcores. 
 When a specific core needs to be stopped, we can use the core's private condition variable.
 This unfortunately has one issue, since we have the private and shared condition variables, we will need extra flags and logic for each fetch-decode-execute
 cycle to determine which condition variable to use which will hamper the efficiency in the long run.
 So unfortunately, we will stop doing that and instead provide another way to block other vcores.
 This was decided with consideration to atomic read and write operations but it seems we will have to implement them at the manager level with one extra 
 function call of abstraction. 
*/

#endif