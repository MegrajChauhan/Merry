#ifndef _MERRY_CONSTS_
#define _MERRY_CONSTS_

// This here defines all the constants that the virtual machine uses
// The ones in the config and utils are different category of contants

#include <merry_types.h>

// Page len: The number of bytes in a page(each page is 1MB is len)
#define _MERRY_PAGE_LEN_ 1048576
#define _MERRY_STACK_PAGE_LEN_ _MERRY_PAGE_LEN_

#define _MERRY_BYTES_PER_PAGE_ _MERRY_PAGE_LEN_
#define _MERRY_WORDS_PER_PAGE_ 524288
#define _MERRY_DWORD_PER_PAGE_ 262144
#define _MERRY_QWORD_PER_PAGE_ 131072

// 1 address represents 8 bytes
#define _MERRY_ADDRESSABLE_ADDRESSES_ON_STACK_ _MERRY_QWORD_PER_PAGE_

typedef struct MerryConsts MerryConsts;

/*Configurable constants*/
struct MerryConsts
{
    msize_t stack_len; // The number of pages for a stack(not the number of bytes)
    // ....
};

#endif