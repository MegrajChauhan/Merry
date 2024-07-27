#include "merry_fio.h"

mret_t merry_open_new_file(mstr_t filepath, msize_t mode, mqptr_t _fd)
{
    if (filepath == NULL)
        return RET_FAILURE;
    int tmp = 0;
#ifdef _USE_LINUX_
    switch (mode)
    {
    case _MERRY_MODE_APPEND_: // create in case doesn't exist
        tmp = open(filepath, _MERRY_OPEN_APPEND_ | _MERRY_CREATE_NEW_, _MERRY_PERMIT_READ_ | _MERRY_PERMIT_WRITE_);
        break;
    case _MERRY_MODE_READ_:
        tmp = open(filepath, _MERRY_OPEN_READ_);
        break;
    case _MERRY_MODE_READ_WRITE_:
        tmp = open(filepath, _MERRY_OPEN_READ_WRITE_ | _MERRY_CREATE_NEW_, _MERRY_PERMIT_READ_ | _MERRY_PERMIT_WRITE_);
        break;
    case _MERRY_MODE_TRUNCATE_:
        tmp = open(filepath, _MERRY_OPEN_TRUNC_ | _MERRY_CREATE_NEW_, _MERRY_PERMIT_READ_ | _MERRY_PERMIT_WRITE_);
        break;
    case _MERRY_MODE_WRITE:
        tmp = open(filepath, _MERRY_OPEN_WRITE_ | _MERRY_CREATE_NEW_, _MERRY_PERMIT_READ_ | _MERRY_PERMIT_WRITE_);
        break;
    default:
        return RET_FAILURE;
    }
    if (tmp == -1)
        return RET_FAILURE;
#elif defined(_USE_WIN_)
 
#endif
    *_fd = tmp;
    return RET_SUCCESS;
}

void merry_close_file(mqptr_t _fd)
{
#ifdef _USE_LINUX_
    close(*_fd);
#else
    // for other systems
#endif
}

msize_t merry_read_from_file(mqptr_t _fd, mbptr_t _store_in, msize_t number_of_bytes)
{
#ifdef _USE_LINUX_
    return read(*_fd, _store_in, number_of_bytes);
#elif defined(_USE_WIN_)
#endif
    return RET_FAILURE;
}

msize_t merry_write_to_file(mqptr_t _fd, mbptr_t src, msize_t num_of_bytes)
{
#ifdef _USE_LINUX_
    return write(*_fd, src, num_of_bytes);
#else
    // for other systems
#endif
    return RET_FAILURE;
}

mret_t merry_seek_in_file(mqptr_t _fd, msize_t _off, msize_t relative_pos)
{
#ifdef _USE_LINUX_
    return lseek(*_fd, _off, relative_pos) == -1 ? RET_FAILURE : RET_SUCCESS;
#else

#endif
    return RET_FAILURE;
}