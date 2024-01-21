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

/*
  Change of plans.
  Since it looks like it will be hell to implement atomic operations at the Manager level, we will have to use something different and inefficient.
  Now each memory page will have a lock of its own which is used for reading and writing.
  Every read and write operation is now atomic at least for the data memory.
  Instruction memory doesn't need atomic reads and writes as the vcores will only be reading from it.
*/

// This module is the backbone of the VM and controls everything
#include "merry_include.h"

typedef struct Merry Merry;

struct Merry
{
  MerryCore **cores;          // the vcores
  MerryThread **core_threads; // the vcore's threads
  MerryMemory *inst_mem;      // the instruction memory that every vcore shares
  MerryMemory *data_mem;      // the data memory that every vcore shares
  MerryMutex *_lock;          // the Manager's lock
  // MerryMutex *_mem_lock;  // lock for memory read/write
  MerryCond *_cond; // the Manager's cond
  // MerryCond *shared_cond; // this condition is shared among all cores
  msize_t core_count; // the number of vcores
  mbool_t stop;       // tell the manager to stop the VM and exit
  msize_t ret;
};


#define _MERRY_REQUEST_QUEUE_LEN_ 10 // for now

#define _MERRY_REQUEST_INTERNAL_ERROR_(request_id) (request_id > 0 && request_id <= 50)
#define _MERRY_REQUEST_PROGRAM_ERROR_(request_id) (request_id >= 51 && request_id <= 150)

#define merry_mem_error(msg) fprintf(stderr, "Memory Error: %s.\n", msg)
#define merry_internal_module_error(msg) fprintf(stderr, "Internal Error; %s.\n", msg)
/*
 The Manager assigns core ids to every core it manages which then helps in identifying the cores.
*/

// this only initializes an instance of Merry while leaving inst_mem, data_mem uninitialized which is valid as we need to know the input and how many
// pages to start with

static Merry os;

// directly used by cores
#define merry_manager_mem_read_inst(inst_mem, address, store_in) merry_memory_read(inst_mem, address, store_in)
#define merry_manager_mem_read_data(address, store_in) merry_memory_read_lock(os.data_mem, address, store_in)
#define merry_manager_mem_write_data(address, _value_to_write) merry_memory_write_lock(os.data_mem, address, _value_to_write)

mret_t merry_os_init(mcstr_t _inp_file);

mptr_t merry_os_start_vm(mptr_t some_arg);

// destroy the OS
void merry_os_destroy();

mret_t merry_os_mem_read_data(maddress_t address, mqptr_t store_in, msize_t core_id);

mret_t merry_os_mem_write_data(maddress_t address, mqword_t to_store, msize_t core_id);

// print the suitable error message and exit the VM
void merry_os_handle_error(merrot_t error);

void merry_os_handle_internal_module_error(merrot_t error_num);

#endif