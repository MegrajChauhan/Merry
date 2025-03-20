#include "merry_helpers.h"

mbool_t str_starts_with(mstr_t src, mstr_t _prefix)
{
    msize_t srclen = strlen(src);
    msize_t _prefixlen = strlen(_prefix);
    if (_prefixlen > srclen)
        return mfalse;
    if (_prefixlen == srclen && strcmp(src, _prefix) == 0)
        return mtrue;
    if (strncmp(src, _prefix, _prefixlen) == 0)
        return mtrue;
    return mfalse;
}

mbool_t str_ends_with(mstr_t src, mstr_t _suffix)
{
    msize_t srclen = strlen(src);
    msize_t _suffixlen = strlen(_suffix);
    if (_suffixlen > srclen)
        return mfalse;
    if (_suffixlen == srclen && strcmp(src, _suffix) == 0)
        return mtrue;
    if (strncmp(src + (srclen - _suffixlen), _suffix, _suffixlen) == 0)
        return mtrue;
    return mfalse;
}