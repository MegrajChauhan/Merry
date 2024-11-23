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
#ifndef _MERRY_SYSCALL_HDLR_
#define _MERRY_SYSCALL_HDLR_

/**
 * We will not handle every syscall. The syscalls that we will handle are:
 * - Have pointers to array of bytes as parameters.
 * - Affect the execution of VM in a negative way.
 */

#include "merry_config.h"
#include "merry_types.h"
#include "merry_core.h"
#include "merry_request_hdlr.h"
#include "merry_console.h"
#include <stdlib.h>
#ifdef _USE_LINUX_
#include <syscall.h>
#include <unistd.h>
#endif

#define merry_syscall(name) void merry_syscall_##name (MerryCore *c)

merry_syscall(open);
merry_syscall(read);
merry_syscall(write);

/**
 * Ma = syscall call number
 * M1 = arg1
 * M2 = arg2
 * M3 = arg3
 * M4 = arg4
 * M5 = arg5
 */
void merry_exec_syscall(MerryCore *c);

#endif