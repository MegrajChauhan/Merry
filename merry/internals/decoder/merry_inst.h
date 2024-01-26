/*
 * Instruction representation of the Merry VM
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
#ifndef _MERRY_INST_
#define _MERRY_INST_

typedef struct MerryInstruction MerryInstruction;

#include "merry_opcodes.h"

// A function that executes the instruction should take a MerryInstruction *, MerryCore *
// Any error generated during this  will be reported by the executing function itself
_MERRY_DEFINE_FUNC_PTR_(void, minstexec_t, struct MerryCore *)

struct MerryInstruction
{
    mopcode_t opcode; // the instruction's opcode
    // based on the instruction, it may have many operands, but at most it can have only 2 operands
    moperand_t op1;        // the first operand
    moperand_t op2;        // the second operand
    minstexec_t exec_func; // the function that executes this specific instruction
};

#endif