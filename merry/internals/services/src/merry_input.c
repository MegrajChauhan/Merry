#include "../merry_input.h"

_MERRY_ALWAYS_INLINE_ void merry_read_char(mptr_t _store_in)
{
    *(mstr_t)_store_in = getchar(); // read a character
}

msize_t merry_readline(mstr_t _num, msize_t len)
{
    // _num will be originally NULL
    _num = (mstr_t)malloc(len);
    if (_num == NULL)
        return 0; // failed
    msize_t read_count = 0;
    for (;; read_count++)
    {
        // read until
        if (read_count == len)
        {
            // _num is already full
            // reallocation needed
            /// TODO: Give a damn about possible reallocation failure
            _num = (mstr_t)realloc(_num, len * 2);
            len *= 2;
        }
        _num[read_count] = getchar();
        if (_num[read_count] == '\n')
        {
            // we have met with the legendary newline
            // newline is replaced by '\0'
            _num[read_count] = '\0';
            break;
        }
        else
        {
            read_count++;
        }
    }
    return read_count;
}

_MERRY_ALWAYS_INLINE_ void merry_read_word(mptr_t _store_in)
{
    fscanf(stdin, "%hu", (mword_t *)_store_in);
}

_MERRY_ALWAYS_INLINE_ void merry_read_dword(mptr_t _store_in)
{
    fscanf(stdin, "%u", (mword_t *)_store_in);
}

_MERRY_ALWAYS_INLINE_ void merry_read_qword(mptr_t _store_in)
{
    fscanf(stdin, "%llu", (mword_t *)_store_in);
}

