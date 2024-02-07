// #ifndef _MERRY_INPUT_
// #define _MERRY_INPUT_

// #include "../../../utils/merry_types.h"
// // #include <string.h>
// #include <stdio.h>
// #include <stdlib.h>

// #define _MERRY_BASE_LEN_ 32

// // the simplest one
// void merry_read_char(mptr_t _store_in); // _store_in is an address in the data_mem that the manager will provide

// // implementing the above is all that is needed
// // reading integers and strings can be implemented with the above as the base

// // This function is for this module specifically but could be useful in the future
// msize_t merry_readline(mstr_t _num, msize_t len); /*Read until newline, allocate necessary memory as needed if len is not enough*/

// // this is going to read a number of 2 bytes long
// void merry_read_word(mptr_t _store_in);

// // this is going to read a number of 4 bytes long
// void merry_read_dword(mptr_t _store_in);

// // this is going to read a number of 8 bytes long
// void merry_read_qword(mptr_t _store_in);

// #endif