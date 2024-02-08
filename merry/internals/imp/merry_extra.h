#ifndef _MERRY_EXTRA_
#define _MERRY_EXTRA_

#include "../../../utils/merry_types.h"

extern inline void _mem_read_word_(mbptr_t src, mqptr_t dest);
extern inline void _mem_read_dword_(mbptr_t src, mqptr_t dest);
extern inline void _mem_read_qword_(mbptr_t src, mqptr_t dest);

#endif