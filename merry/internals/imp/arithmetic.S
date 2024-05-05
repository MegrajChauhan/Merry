/*
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

.intel_syntax noprefix
#include "../../../utils/merry_config.h"

// We believe that every machine that Merry runs on uses 2's complement for negative number representation and
// IEEE representation for floating point numbers

// void _update_flags_(MerryFlagRegister *reg)
.global _update_flags_
// void _cmp_inst_(mqword_t val1, mqword_t val2, struct MerryFlagRegsiter *reg)
.global _cmp_inst_


#if defined(_MERRY_HOST_CPU_x86_64_ARCH_)
_update_flags_:
   pushfq          // push the flags register unto the stack
   pop rax        // pop the flags
   mov [rdi], rax // save the values back such that we have the flags value
   ret

#if defined(_MERRY_HOST_OS_LINUX_)
_cmp_inst_:
   cmp rdi, rsi
   mov rdi, rdx
   call _update_flags_
   ret
#endif

#if defined(_MERRY_HOST_OS_WINDOWS_)
_cmp_inst_:
   cmp rcx, rdx
   mov rdi, r8
   call _update_flags_
   ret
#endif

#endif