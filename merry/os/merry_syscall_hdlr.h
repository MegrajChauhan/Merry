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