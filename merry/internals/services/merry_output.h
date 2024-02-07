#ifndef _MERRY_OUTPUT_
#define _MERRY_OUTPUT_

#include "../../../utils/merry_types.h"
#include <stdio.h>

void merry_write_char(mptr_t _to_write);

void merry_write_word(mptr_t _to_write);

void merry_write_dword(mptr_t _to_write);

void merry_write_qword(mptr_t _to_write);

void merry_write_bytes(mptr_t _to_write, msize_t _len);

#endif