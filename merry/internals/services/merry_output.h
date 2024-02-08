#ifndef _MERRY_OUTPUT_
#define _MERRY_OUTPUT_

#include "../../../utils/merry_types.h"
#include <stdio.h>
#include "../merry_dmemory.h"

mret_t merry_write_char(MerryDMemory *mem, maddress_t address);

// void merry_write_word(mptr_t _to_write);

// void merry_write_dword(mptr_t _to_write);

// void merry_write_qword(mptr_t _to_write);

// void merry_write_bytes(mptr_t _to_write, msize_t _len);

#endif