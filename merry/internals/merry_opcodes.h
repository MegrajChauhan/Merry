/*
 * The opcode definition of the Merry VM
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
#ifndef _MERRY_OPCODES_
#define _MERRY_OPCODES_

/*
 The structure of an instruction:
 We need opcodes and operands. For operands that are 64 bit long, they should be right after the instruction in memory.
 Opcodes are 9 bits long, for now atleast.
 This provides us with 512 possible instructions. If we need to expand then we can do so in the future.
 For now, 9 bits is enough.
 This leaves us with 55 bits that can be used for the operands.
 Since we have a lot of possible opcodes, we will not make use of any flags and instead use different variants for the same instruction
*/

#if defined(_WIN64)
#include "..\..\utils\merry_config.h"
#else
#include "../../utils/merry_config.h"
#endif

enum
{
  OP_NOP,  // no operation instruction
  OP_HALT, // halt instruction

  /*Firstly: The basic arithmetic instructions*/
  // The arithmetic instructions have two variants: One with IMM/DEST and another with SRC/DEST
  OP_ADD_IMM,
  OP_ADD_REG,
  OP_SUB_IMM,
  OP_SUB_REG,
  OP_MUL_IMM,
  OP_MUL_REG,
  OP_DIV_IMM,
  OP_DIV_REG,
  OP_MOD_IMM,
  OP_MOD_REG,

  // The signed versions of the above instructions
  OP_IADD_IMM,
  OP_IADD_REG,
  OP_ISUB_IMM,
  OP_ISUB_REG,
  OP_IMUL_IMM,
  OP_IMUL_REG,
  OP_IDIV_IMM,
  OP_IDIV_REG,
  OP_IMOD_IMM,
  OP_IMOD_REG,

  // as for floating point numbers
  // Merry currently assumes that the bits of the operands are in IEEE format and that the processor also uses IEEE format for execution
  // these instructions related to floating point numbers will be added soon enough

  // floating point instructions only take register operands
  // These below ones are 64-bit i.e double precision
  OP_FADD,
  OP_FSUB,
  OP_FMUL,
  OP_FDIV,

  // Now move intructions
  OP_MOVE_IMM = 26, // move an immediate value to a destination register[This is default and accepts 32 bit values]

  OP_MOVE_IMM_64, // move an immediate value to a destination register[This is a variant and accepts 64 bit value that must follow the inst in the memory]

  OP_MOVE_REG, // move a value from one register to another

  OP_MOVE_REG8,    // move the lowest byte of one register to another
  OP_MOVE_REG16,   // move the lowest two bytes of one register to another
  OP_MOVE_REG32,   // move the lowest 4 bytes of one register to another
  OP_MOVESX_IMM8,  // move a value and sign extend it as well[only 8-bit numbers are accepted]
  OP_MOVESX_IMM16, // move a value and sign extend it as well[only 16-bit numbers are accepted]

  OP_MOVESX_IMM32, // move a value and sign extend it as well[only 32-bit numbers are accepted]

  OP_MOVESX_REG8,  // move the lower 1 byte to another register while sign extending it
  OP_MOVESX_REG16, // move the lower 2 bytes to another register while sign extending it

  OP_MOVESX_REG32, // move the lower 4 bytes to another register while sign extending it
  // we won't need zero extend. It can simply be aliased with simple move

  // Control flow instructions
  // the JMP instruction takes only a maximum of 40 bits from the instruction as the address
  // either an offset is to be provided or the address can be provided as well
  OP_JMP_OFF,  // JMP inst but the offset from current PC is provided[If the offset provided is in 2's complement then we can jump back]
  OP_JMP_ADDR, // JMP inst but the address is directly provided
  OP_CALL,     // the call instruction
  OP_RET,      // return from a call
  OP_SVA,      // access variables on the stack
  OP_SVC,      // change the variable's value on the stack

  // conditional jmps once we have a branch predictor

  // Now some stack operations
  OP_PUSH_IMM, // the push instruction that pushes an immediate
  OP_PUSH_REG, // push a register
  OP_POP,      // pop a value to a register
  OP_PUSHA,    // push all the registers {The order is from Ma through Mm5 linearly}
  OP_POPA,     // pop to all registers in the reverse order

  // logical instructions
  OP_AND_IMM, // REG & IMM
  OP_AND_REG, // REG & REG
  OP_OR_IMM,  // REG | TMM
  OP_OR_REG,  // REG | REG
  OP_XOR_IMM, // REG ^ IMM
  OP_XOR_REG, // REG ^ REG
  OP_NOT,     // ~REG
  OP_LSHIFT,  // REG << <num>
  OP_RSHIFT,  // REG >> <num>
  OP_CMP_IMM, // CMP REG and IMM
  OP_CMP_REG, // CMP REG and REG

  // additional arithmetic instructions
  // These will not affect the flags register at all
  OP_INC,
  OP_DEC,

  // data movement instructions
  /*
    LEA will require many operands.
    The first operand is the base address which can be any register.
    The second operand is the index which can also be in any register
    The third operand is the scale which can also be in any register
    The last is the destination register which can also be any register
    dest = base + index * scale [Useful for arrays: Doesn't change flags]
  */
  OP_LEA,
  /*
     The move instructions do not allow accessing the data memory for data and so that ability will be provided by LOAD.
     The VM works with 8 bytes at once and so if the program has compressed data for multiple variables into those 8 bytes then the use of logical shifts should help
  */
  OP_LOAD,
  OP_STORE,

  // as the move_regX and move_immX instructions overwrite the bytes not being written to, the excg instruction will mitigate that problem.
  // This instruction will only work with registers and not immediates
  // One issue here is that the values between the registers are exchanged
  OP_EXCG8,  // exchange only 1 lower byte
  OP_EXCG16, // exchange only 2 lower bytes
  OP_EXCG32, // exchange only 4 lower bytes
  OP_EXCG,   // exchange the entire values

  // To MOVE values instead of exchanging, we use the movX instruction that moves the bytes without overwriting anything
  // This also only works on registers
  // we don't need the 64-bit version
  OP_MOV8,
  OP_MOV16,
  OP_MOV32,

  // some utility instructions
  OP_CFLAGS, // clear the flags register
  OP_RESET,  // reset all the registers
  OP_CLZ,    // clear the zero flag
  OP_CLN,    // clear the negative flag
  OP_CLC,    // clear the carry flag
  OP_CLO,    // clear the overflow flag

  // conditional jumps
  OP_JNZ,
  OP_JZ,
  OP_JNE,
  OP_JE,
  OP_JNC,
  OP_JC,
  OP_JNO,
  OP_JO,
  OP_JNN,
  OP_JN,
  OP_JNG,
  OP_JG,
  OP_JNS,
  OP_JS,
  OP_JGE,
  OP_JSE,

  // conditional data movement instructions will be implemented in the future accordingly with the needs

  // some uitlity instructions
  OP_LOOP, // automatically jumps to the given address until the Mc register is 0

  OP_INTR,

  // Some more
  OP_LOADB,  // load byte
  OP_LOADW,  // load 2 bytes
  OP_LOADD,  // load 4 bytes
  OP_STOREB, // store byte
  OP_STOREW, // store 2 bytes
  OP_STORED, // store 4 bytes

  // the addresses are in register
  OP_LOAD_REG,
  OP_STORE_REG,
  OP_LOADB_REG,
  OP_STOREB_REG,
  OP_LOADW_REG,
  OP_STOREW_REG,
  OP_LOADD_REG,
  OP_STORED_REG,

  // helping instructions
  OP_CMPXCHG, // the atomic compare and exchange instruction

  // Some other instructions
  // IO through interrupts is also allowed
  // This is also a way to do so
  OP_CIN,  // character input
  OP_COUT, // character output
  OP_SIN,  // input a given number of bytes[characters or a string][string in]
  OP_SOUT, // output a given number of bytes[As characters][string out]

  OP_IN,   // input a byte
  OP_OUT,  // print a byte
  OP_INW,  // input a word[signed]
  OP_OUTW, // output a word[signed]
  OP_IND,  // input a dword[signed]
  OP_OUTD, // output a dword[signed]
  OP_INQ,  // input a qword[signed]
  OP_OUTQ, // output a qword[signed]

  OP_UIN,   // input a byte[unsigned]
  OP_UOUT,  // output a byte[unsigned]
  OP_UINW,  // input a word[unsigned]
  OP_UOUTW, // output a word[unsigned]
  OP_UIND,  // input a dword[unsigned]
  OP_UOUTD, // output a dword[unsigned]
  OP_UINQ,  // input a qword[unsigned]
  OP_UOUTQ, // output a qword[unsigned]

  OP_INF,  // read a 64-bit float
  OP_OUTF, // print a 64-bit float

  OP_INF32,  // read a 32-bit float
  OP_OUTF32, // print a 32-bit float

  OP_OUTR,  // print all register's contents as signed values[takes no operands]
  OP_UOUTR, // print all register's contents as unsigned values[takes no operands]

  // for 32-bit floating point numbers
  OP_FADD32,
  OP_FSUB32,
  OP_FMUL32,
  OP_FDIV32,

  OP_ATOMIC_LOAD,   // load 8 bytes atomically
  OP_ATOMIC_LOADB,  // load byte atomically
  OP_ATOMIC_LOADW,  // load 2 bytes atomically
  OP_ATOMIC_LOADD,  // load 4 bytes atomically
  OP_ATOMIC_STORE,  // store 8 bytes atomically
  OP_ATOMIC_STOREB, // store byte atomically
  OP_ATOMIC_STOREW, // store 2 bytes atomically
  OP_ATOMIC_STORED, // store 4 bytes atomically

};

/*
 This is what the stack looks like:
 ADDR VALUE
    0   00
    1   10 <- Old BP position
    2   ff
    3   ad  <- "svc 2 <src>" will put whatever was in <src> to 3
    4   ee  <- Variable[Access using "sva <dest> 1"]
    5   01  <- BP[BP's old value is saved here and it is pointing to it]
    6   ..  <- SP points here
    7   ..
    .
    sva <dest> 1 returns the value at BP - 1 and puts it into register <dest>
    As long as the offset is valid, the requested value is provided
*/

// operands are basically numbers which represent different things based on the instruction
typedef unsigned long long moperand_t;

#define merry_get_opcode(inst) (inst >> 56)
#define merry_get_opcodel(inst) (inst >> 48)

#endif