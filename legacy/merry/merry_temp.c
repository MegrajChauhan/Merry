#include "merry_temp.h"

mret_t lock_file(FILE *file)
{
#ifdef _USE_WIN_
    HANDLE h = (HANDLE)_get_osfhandle(_fileno(file));
    if (h == INVALID_HANDLE_VALUE)
    {
        inlog("File Lock Failed");
        return RET_FAILURE;
    }
    if (!LockFile(h, 0, 0, 1, 0))
    {
        inlog("File Lock Failed");
        return RET_FAILURE; // Lock failed
    }
#else
    int fd = fileno(file);
    if (flock(fd, LOCK_EX) == RET_FAILURE)
    {
        inlog("File Lock Failed");
        return RET_FAILURE;
    }
#endif
    inlog("File Locked Successfully");
    return RET_SUCCESS;
}

mret_t unlock_file(FILE *file)
{
#ifdef _USE_WIN_
    HANDLE h = (HANDLE)_get_osfhandle(_fileno(file));
    if (h == INVALID_HANDLE_VALUE)
    {
        inlog("File UnLock Failed");
        return RET_FAILURE;
    }
    if (!UnlockFile(h, 0, 0, 1, 0))
    {
        inlog("File UnLock Failed");
        return RET_FAILURE;
    }
#else
    int fd = fileno(file);
    if (flock(fd, LOCK_UN) == -1)
    {
        inlog("File UnLock Failed");
        return RET_FAILURE;
    }
#endif
    inlog("File UnLocked Successfully");
    return RET_SUCCESS;
}