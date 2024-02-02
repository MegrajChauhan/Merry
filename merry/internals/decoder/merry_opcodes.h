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
    // 5 opcodes will be reserved for floating point instructions

    // Now move intructions
    OP_MOVE_IMM = 26, // move an immediate value to a destination register[This is default and accepts 32 bit values]
    OP_MOVE_IMM_64,   // move an immediate value to a destination register[This is a variant and accepts 64 bit value that must follow the inst in the memory]
    OP_MOVE_REG,      // move a value from one register to another
    OP_MOVE_REG8,     // move the lowest byte of one register to another
    OP_MOVE_REG16,    // move the lowest two bytes of one register to another
    OP_MOVE_REG32,    // move the lowest 4 bytes of one register to another
    OP_MOVESX_IMM8,   // move a value and sign extend it as well[only 8-bit numbers are accepted]
    OP_MOVESX_IMM16,  // move a value and sign extend it as well[only 16-bit numbers are accepted]
    OP_MOVESX_IMM32,  // move a value and sign extend it as well[only 32-bit numbers are accepted]
    OP_MOVESX_REG8,   // move the lower 1 byte to another register while sign extending it
    OP_MOVESX_REG16,  // move the lower 2 bytes to another register while sign extending it
    OP_MOVESX_REG32,  // move the lower 4 bytes to another register while sign extending it
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
    // The move instruction doesn't allow movement from 
    OP_LOAD, 
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
#define merry_get_opcodel(inst) (int >> 48)

/*
    // conditional moves don't have the same variations like the unconditional moves
    // conditional moves
    ASP_MOVEZ_IMM, // move if zero flag is set
    ASP_MOVEZ_REG, // move if zero flag is set

    ASP_MOVENZ_IMM, // move if zero flag is not set
    ASP_MOVENZ_REG, // move if zero flag is not set

    ASP_MOVEE_IMM, // move if equal flag is set
    ASP_MOVEE_REG, // move if equal flag is set

    ASP_MOVENE_IMM, // move if equal flag is not set
    ASP_MOVENE_REG, // move if equal flag is not set

    ASP_MOVEG_IMM, // move if greater flag is set
    ASP_MOVEG_REG, // move if greater flag is set

    ASP_MOVES_IMM, // move if greater flag is not set
    ASP_MOVES_REG, // move if greater flag is not set

    ASP_MOVEO_IMM, // move if overflow flag is set
    ASP_MOVEO_REG, // move if overflow flag is set

    ASP_MOVENO_IMM, // move if overflow flag is not set
    ASP_MOVENO_REG, // move if overflow flag is not set

    ASP_MOVEN_IMM, // move if negative flag is set
    ASP_MOVEN_REG, // move if negative flag is set

    ASP_MOVENN_IMM, // move if negative flag is not set
    ASP_MOVENN_REG, // move if negative flag is not set

    ASP_MOVEGE_IMM, // move if greater or equal flag is set
    ASP_MOVEGE_REG, // move if greater or equal flag is set

    ASP_MOVESE_IMM, // move if greater is not set or equal flag is set
    ASP_MOVESE_REG, // move if greater is not set or equal flag is set

    ASP_MOVEC_IMM, // move if carry is set
    ASP_MOVEC_REG, // move if carry is set

    ASP_MOVENC_IMM, // move if carry is not set
    ASP_MOVENC_REG, // move if carry is not set

    ASP_EXCG8,  // exchange instruction that exchanges the values between two registers[8 bits]
    ASP_EXCG16, // exchange instruction that exchanges the values between two registers[16 bits]
    ASP_EXCG32, // exchange instruction that exchanges the values between two registers[32 bits]
    ASP_EXCG,   // exchange instruction that exchanges the values between two registers[64 bits

    // conditional jumps
    ASP_JZ,  // jump if zero
    ASP_JNZ, // jump if not zero
    ASP_JE,  // jump if equal
    ASP_JNE, // jump if not equal
    ASP_JG,  // jump if greater
    ASP_JS,  // jump if not greater
    ASP_JO,  // jump if overflow
    ASP_JNO, // jump if no overflow
    ASP_JN,  // jump if negative
    ASP_JNN, // jump if no negative
    ASP_JGE, // jump if greater or equal
    ASP_JSE, // jump if smaller or equal

    ASP_RESET, // the reset instruction[resets all the registers except program counter, bp, sp to zero. resets flags to zero as well.]

    ASP_CFLAGS, // clear the flag register
    ASP_CLZ,    // clear the zero flag
    ASP_CLN,    // clear the negative flag
    ASP_CLE,    // clear the equal flag
    ASP_CLG,    // clear the greater flag
    ASP_CLO,    // clear the overflow flag
    ASP_CLC,    // clear the carrt flag

    // memory related instructions
    // These instructions only interact with data memory and not with the instruction memory
    ASP_LOAD,  // load instruction to load from memory to a given register
    ASP_STORE, // store instruction to store to memory from a given register

    ASP_INTR, // generate an interrupt
*/

#endif