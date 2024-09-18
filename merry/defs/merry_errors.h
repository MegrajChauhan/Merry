/*
 * Error representation for the Merry VM
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

#ifndef _MERRY_ERRORS_
#define _MERRY_ERRORS_

// this includes all of the internal errors that may be produced

/*The call depth will be set according to how much the stack can handle*/
enum
{
    MERRY_ERROR_NONE,
    _PANIC_REQBUFFEROVERFLOW = 1,
    _SHOULD_EXIT = 10,             // We received the order to terminate from the OS                  
    MERRY_MEM_ACCESS_ERROR = 51,   // accessing the memory in a wrong way
    MERRY_MEM_INVALID_ACCESS,      // indicating memory access for memory addresses that either do not exist or are invalid
    MERRY_DIV_BY_ZERO,             // dividing by zero
    MERRY_STACK_OVERFLOW,          // stack is overflowing
    MERRY_STACK_UNDERFLOW,         // the stack has less than expected number of values
    MERRY_CALL_DEPTH_REACHED,      // reached the limit of function calls
    MERRY_INVALID_RETURN,          // invalid return instruction
    MERRY_INVALID_VARIABLE_ACCESS, // accessing variable in the wrong way on the stack
    MERRY_DYNL_FAILED,             // dynamic loading failed
    MERRY_DYNCALL_FAILED,          // couldn't call any function
    MERRY_INTERNAL_ERROR,          // this error occured such that the requester is unable to deduce the cause
};

#endif