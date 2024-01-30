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

    // The signed versions of the above instructions
    OP_IADD_IMM,
    OP_IADD_REG,
    OP_ISUB_IMM,
    OP_ISUB_REG,
    OP_IMUL_IMM,
    OP_IMUL_REG,
    OP_IDIV_IMM,
    OP_IDIV_REG,

    // as for floating point numbers
    // Merry currently assumes that the bits of the operands are in IEEE format and that the processor also uses IEEE format for execution

};

typedef unsigned int mopcode_t; /*Opcode*/
// operands are basically numbers which represent different things based on the instruction
typedef unsigned long long moperand_t;

#define merry_get_opcode(inst) (inst >> 55)

#endif