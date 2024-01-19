#ifndef _MERRY_ERRORS_
#define _MERRY_ERRORS_

// this includes all of the internal errors that may be produced
enum
{
    _PANIC_REQBUFFEROVERFLOW,
    MERRY_MEM_ACCESS_ERROR = 51, // accessing the memory in a wrong way
    MERRY_MEM_INVALID_ACCESS,    // indicating memory access for memory addresses that either do not exist or are invalid
};

#endif