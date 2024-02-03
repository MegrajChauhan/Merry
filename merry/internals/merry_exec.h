#ifndef _MERRY_EXEC_
#define _MERRY_EXEC_

#include "../../utils/merry_logger.h"
#include "merry_request.h"
#include "imp/merry_imp.h"

struct MerryCore;

// The function structure
#define _exec_(name) void merry_execute_##name(struct MerryCore *core)

#define _sign_extend8_(val) val | 0xFFFFFFFFFFFFFF00
#define _sign_extend16_(val) val | 0xFFFFFFFFFFFF0000
#define _sign_extend32_(val) val | 0xFFFFFFFFFF000000

_exec_(nop);

// execute the halt instruction
_exec_(halt);

// arithmetic instructions
_exec_(add_imm);
_exec_(add_reg);
_exec_(sub_imm);
_exec_(sub_reg);
_exec_(mul_imm);
_exec_(mul_reg);
_exec_(div_imm);
_exec_(div_reg);
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

// move instructions
_exec_(move_imm);
_exec_(move_reg);
_exec_(move_reg8);
_exec_(move_reg16);
_exec_(move_reg32);

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

// stack based instructions
_exec_(push_imm);
_exec_(push_reg);
_exec_(pop);
_exec_(pusha);
_exec_(popa);

// logical instructions
// All AND, OR, XOR and CMP take 64 bits values that should follow the instruction in memory
_exec_(and_imm);
_exec_(and_reg);
_exec_(or_imm);
_exec_(or_reg);
_exec_(xor_imm);
_exec_(xor_reg);
_exec_(not );
_exec_(lshift);
_exec_(rshift);
_exec_(cmp_imm);
_exec_(cmp_reg);

// some extra instructions
_exec_(inc);
_exec_(dec);

// data movement instructions
_exec_(lea);
_exec_(load);
_exec_(store);

_exec_(excg);
_exec_(excg8);
_exec_(excg16);
_exec_(excg32);

_exec_(mov8);
_exec_(mov16);
_exec_(mov32);

// utility instructions
#define _clear_(f) core->flag.f = 0

_exec_(cflags);
_exec_(reset);
_exec_(clz);
_exec_(cln);
_exec_(clc);
_exec_(clo);

// contidional jumps
_exec_(jnz);

#endif