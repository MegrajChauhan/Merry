#include "../utils/strutils.h"

int ends_with(const char *src, const char *suffix)
{
    if (src == NULL || suffix == NULL)
        return 1;
    size_t suffix_len = strlen(suffix);
    size_t src_len = strlen(src);
    if (suffix_len > src_len)
        return 1;
    char *suff = src + (src_len - suffix_len);
    if (strcmp(suff, suffix) == 0)
        return 0;
    return 1;
}