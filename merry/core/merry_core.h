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
#include <merry_internals.h>
#include <merry_request.h>
#include <merry_imp.h>
#include <merry_memory.h>
#include <merry_dmemory.h>
#include <merry_opcodes.h>
#include <merry_stack.h>
#include <merry_thread.h>
#include "merry_errno.h"

#if defined(_USE_LINUX_)
#include <unistd.h>
#endif

typedef struct MerryCore MerryCore;
typedef struct MerryFlagRegister MerryFlagRegister;
typedef union F32 F32;
typedef union F64 F64;

#define flags_res(x, size) unsigned long x : size

#define _is_stack_full_(core) (core->sp != (mqword_t)(-1) && core->sp >= _MERRY_MEMORY_QS_PER_PAGE_)
#define _check_stack_lim_(core, size) (core->sp == (mqword_t)(-1) || (_MERRY_MEMORY_QS_PER_PAGE_ - core->sp) > size)
#define _is_stack_empty_(core) (core->sp == (mqword_t)(-1))
#define _stack_has_atleast_(core, atleast) (core->sp >= (atleast - 1))

#define _MERRY_RAS_LEN_ 80             // 80 function calls should be enough
#define _MERRY_RAS_LIMIT_ 160          // 160 function calls at max
#define _MERRY_RAS_GROW_PER_RESIZE_ 20 // 20 new possible function calls per resize

union F32
{
    mdword_t _integer;
    float _float;
};

union F64
{
    mqword_t _integer;
    double _double;
};

struct MerryFlagRegister
{
#if defined(_MERRY_HOST_CPU_x86_64_ARCH_)
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
    unsigned long direction : 1; /*11th bit is the DF[NOT REALLY USEFUL YET BUT MAYBE WHEN IMPLEMENTING STRING RELATED INSTRUCTIONS]*/
    unsigned long overflow : 1;  /*10th bit is the OF*/
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
    MerryDMemory *data_mem; // the data memory
    MerryMemory *inst_mem;  // the instruction memory
    // Each address of the stack stores 8 bytes which implies each push or pop pushes and pops 8 bytes
    // there are no need for alignments
    mqptr_t stack_mem;      // the private stack of the core
    mqptr_t registers;      // the core's registers
    mqword_t sp, bp, pc;    // four registers that is inaccesym = merry_reader_get_symbol(os.reader, c->exception_address)ssible to anything and are changeable indirectly
    mqword_t core_id;       // this register holds the id provided to it which is unique
    MerryFlagRegister flag; // the flags register[This is 64 bits in length. A pointer would be the same length and so there really is no need to declare it as a pointer]
    // some important flags
    mbool_t stop_running; // tell the core to stop executing and shut down
    mbool_t greater;
    mqword_t current_inst;
    MerryStack *ras; // the RAS
    msize_t exception_address;
    mbool_t excp_set;
    msize_t entry_addr;
    MerryStack* callstack;
};

_MERRY_INTERNAL_ _MERRY_ALWAYS_INLINE_ inline void merry_core_zero_out_reg(MerryCore *core)
{
    for (msize_t i = 0; i < REGR_COUNT; i++)
    {
        core->registers[i] = 0;
    }
}

// initialize a new core
MerryCore *merry_core_init(MerryMemory *inst_mem, MerryDMemory *data_mem, msize_t id);

void merry_core_destroy(MerryCore *core, mbool_t _cond);

_THRET_T_ merry_runCore(mptr_t core);

// The function structure
#define _exec_(name) void merry_execute_##name(MerryCore *core)
// some instructions that need full definition
#define _lexec_(name, ...) void merry_execute_##name(MerryCore *core, __VA_ARGS__)

#define _sign_extend8_(val) val | 0xFFFFFFFFFFFFFF00
#define _sign_extend16_(val) val | 0xFFFFFFFFFFFF0000
#define _sign_extend32_(val) val | 0xFFFFFFFFFF000000

#define _clear_(f) core->flag.f = 0
#define _fclear_(f) c->flag.f = 0

#define _LowerTopReg_(current) (current >> 48) & 15
#define _UpperTopReg_(current) (current >> 52) & 15
#define _LowerUpReg_(current) (current >> 4) & 15
#define _LowerDownReg_(current) (current & 15)
#define _Lower4byteImm_(current) (current) & 0xFFFFFFFF

#define _ArithMeticImmFrame_(sign)                                              \
    register mqword_t current = core->current_inst;                             \
    register mqword_t reg = _LowerTopReg_(current);                             \
    core->registers[reg] = core->registers[reg] sign(_Lower4byteImm_(current)); \
    _update_flags_(&core->flag);

#define _SArithMeticImmFrame_(sign)                                                                                   \
    register mqword_t current = core->current_inst;                                                                   \
    register mqword_t reg = _LowerTopReg_(current);                                                                   \
    core->registers[reg] = (msqword_t)core->registers[reg] sign(msqword_t) _sign_extend32_(_Lower4byteImm_(current)); \
    _update_flags_(&core->flag);

#define _ArithMeticRegFrame_(sign)                                                             \
    register mqword_t current = core->current_inst;                                            \
    register mqword_t reg = _LowerUpReg_(current);                                             \
    core->registers[reg] = core->registers[reg] sign core->registers[_LowerDownReg_(current)]; \
    _update_flags_(&core->flag);

#define _SArithMeticRegFrame_(sign)                                                                                  \
    register mqword_t current = core->current_inst;                                                                  \
    register mqword_t reg = _LowerUpReg_(current);                                                                   \
    core->registers[reg] = (msqword_t)core->registers[reg] sign(msqword_t) core->registers[_LowerDownReg_(current)]; \
    _update_flags_(&core->flag);

_MERRY_DEFINE_FUNC_PTR_(mret_t, mem_read, MerryDMemory *, maddress_t, mqptr_t);
_MERRY_DEFINE_FUNC_PTR_(mret_t, mem_write, MerryDMemory *, maddress_t, mqword_t);

// arithmetic instructions
_exec_(add_imm);
_exec_(add_reg);
_exec_(sub_imm);
_exec_(sub_reg);
_exec_(mul_imm);
_exec_(mul_reg);
_exec_(div_imm);
_exec_(div_reg);

_lexec_(add_mem, mem_read func);
_lexec_(sub_mem, mem_read func);
_lexec_(mul_mem, mem_read func);
_lexec_(div_mem, mem_read func);
_lexec_(mod_mem, mem_read func);

_lexec_(cmp_mem, mem_read func);

_exec_(fadd64_mem);
_exec_(fsub64_mem);
_exec_(fmul64_mem);
_exec_(fdiv64_mem);

_exec_(fadd32_mem);
_exec_(fsub32_mem);
_exec_(fmul32_mem);
_exec_(fdiv32_mem);

_exec_(mod_imm);
_exec_(mod_reg);

_exec_(iadd_imm);
_exec_(iadd_reg);
_exec_(isub_imm);
_exec_(isub_reg);
_exec_(imul_imm);
_exec_(imul_reg);
_exec_(idiv_imm);
_exec_(idiv_reg);
_exec_(imod_imm);
_exec_(imod_reg);

_exec_(fadd);
_exec_(fsub);
_exec_(fmul);
_exec_(fdiv);

_exec_(fadd32);
_exec_(fsub32);
_exec_(fmul32);
_exec_(fdiv32);

_exec_(movesx_reg8);
_exec_(movesx_reg16);
_exec_(movesx_reg32);
_exec_(movesx_imm8);
_exec_(movesx_imm16);
_exec_(movesx_imm32);

// control flow instructions
_exec_(call);
_exec_(ret);
_exec_(sva);
_exec_(svc);
_exec_(sva_mem);
_exec_(svc_mem);
_exec_(sss_imm);
_exec_(gss_imm);

// stack based instructions
_exec_(push_imm);
_exec_(push_reg);
_exec_(pop);
_exec_(pusha);
_exec_(popa);

_lexec_(load, mqword_t address);
_lexec_(store, mqword_t address);
_lexec_(loadb, mqword_t address);
_lexec_(storeb, mqword_t address);
_lexec_(loadw, mqword_t address);
_lexec_(storew, mqword_t address);
_lexec_(loadd, mqword_t address);
_lexec_(stored, mqword_t address);
_lexec_(load_reg, mqword_t address);
_lexec_(store_reg, mqword_t address);
_lexec_(loadb_reg, mqword_t address);
_lexec_(storeb_reg, mqword_t address);
_lexec_(loadw_reg, mqword_t address);
_lexec_(storew_reg, mqword_t address);
_lexec_(loadd_reg, mqword_t address);
_lexec_(stored_reg, mqword_t address);

_lexec_(atm_load, mqword_t address);
_lexec_(atm_loadb, mqword_t address);
_lexec_(atm_loadw, mqword_t address);
_lexec_(atm_loadd, mqword_t address);

_lexec_(atm_store, mqword_t address);
_lexec_(atm_storeb, mqword_t address);
_lexec_(atm_storew, mqword_t address);
_lexec_(atm_stored, mqword_t address);

_exec_(excg);
_exec_(excg8);
_exec_(excg16);
_exec_(excg32);

_exec_(syscall);

_lexec_(push_mem, mem_read func);
_lexec_(pop_mem, mem_write func);

#endif