#include "merry_temp.h"

mret_t lock_file(FILE *file)
{
#ifdef _USE_WIN_
    HANDLE h = (HANDLE)_get_osfhandle(_fileno(file));
    if (h == INVALID_HANDLE_VALUE)
    {
        return RET_FAILURE;
    }

    if (!LockFile(h, 0, 0, 1, 0))
        return RET_FAILURE; // Lock failed
#else
    int fd = fileno(file);
    if (flock(fd, LOCK_EX) == RET_FAILURE)
        return RET_FAILURE;
#endif
    return RET_SUCCESS;
}

mret_t unlock_file(FILE *file)
{
#ifdef _USE_WIN_
    HANDLE h = (HANDLE)_get_osfhandle(_fileno(file));
    if (h == INVALID_HANDLE_VALUE)
        return RET_FAILURE;
    if (!UnlockFile(h, 0, 0, 1, 0))
        return RET_FAILURE; // Unlock failed
#else
    int fd = fileno(file);
    if (flock(fd, LOCK_UN) == -1)
        return RET_FAILURE;
#endif
    return RET_SUCCESS;
}