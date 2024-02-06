#include "../merry_output.h"

_MERRY_ALWAYS_INLINE_ void merry_write_char(mptr_t _to_write)
{
   fprintf(stdout, "%c", *(mstr_t)_to_write);
}

_MERRY_ALWAYS_INLINE_ void merry_write_word(mptr_t _to_write)
{
   fprintf(stdout, "%hu", *(mstr_t)_to_write);
}

_MERRY_ALWAYS_INLINE_ void merry_write_dword(mptr_t _to_write)
{
   fprintf(stdout, "%u", *(mstr_t)_to_write);
}

_MERRY_ALWAYS_INLINE_ void merry_write_qword(mptr_t _to_write)
{
   fprintf(stdout, "%llu", *(mstr_t)_to_write);
}

_MERRY_ALWAYS_INLINE_ void merry_write_bytes(mptr_t _to_write, msize_t _len)
{
   // _len includes the terminating character as well
   // we will replace the _len position with '\0\ ourselves
   *(mstr_t)(_to_write + _len) = '\0';
   fprintf(stdout, "%s", (mcstr_t)_to_write);
}
