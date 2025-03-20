#ifndef _MERRY_ERRNO_
#define _MERRY_ERRNO_

// This is a pretty useless module(need to reuse this somehow)

/*This module isn't being used at all actually.*/

#include <errno.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <merry_console.h>

/*
Since each platform has different number of errno values, we have to be really careful to deal with them.
*/

/**
 * The problem we have:
 * Each platform has it's own set of ERRNO values and so to support different platform means
 * supporting every single ERRNO value, which is pure absurdity.
 */
#define MERRY_NODBG 0       // Debugging was enabled for the child but the debugger wasn't connected at all
#define MERRY_SYSCALLERR 1  // The syscall being handled by the OS didn't expect the provided argument
#define MERRY_DYNERR 2      // The recent dynamic library related request failed
#define MERRY_DYNCLOSED 3   // The handle has already been closed
#define MERRY_INIT_FAILED 4 // Failed to initialize something
#define MERRY_INVAL_ARG 5   // invalid argument
#define MERRY_SYSERR 6      // error that was caused by the host system itself
#define MERRY_VMERR 7       // the error was due to the VM

// Our errno value
_MERRY_INTERNAL_ msize_t merrno = (msize_t)-1;

// This will reset errno
msize_t merry_get_errno(void);

void merry_set_errno(msize_t _err);

#endif