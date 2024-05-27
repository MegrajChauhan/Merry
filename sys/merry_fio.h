#ifndef _MERRY_FIO_
#define _MERRY_FIO_

#if defined(_WIN64)
#include "..\utils\merry_types.h"
#else
#include "../utils/merry_types.h"
#endif

#if defined(_MERRY_HOST_OS_LINUX_)
#define _USE_FLINUX_
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#define _MERRY_OPEN_READ_ O_RDONLY
#define _MERRY_OPEN_WRITE_ O_WRONLY
#define _MERRY_OPEN_READ_WRITE_ O_RDWR
#define _MERRY_OPEN_APPEND_ O_APPEND
#define _MERRY_OPEN_TRUNC_ O_TRUNC // truncate
#define _MERRY_CREATE_NEW_ O_CREAT

// Permissions for when creating a new file
#define _MERRY_PERMIT_READ_ S_IRUSR
#define _MERRY_PERMIT_WRITE_ S_IWUSR
// we don't need the execute permissions

#elif defined(_MERRY_HOST_OS_WINDOWS_)
#define _USE_FWIN_
#include <windows.h>
#endif

#define _MERRY_MODE_READ_ 0
#define _MERRY_MODE_WRITE 1     // If doesn't exist, create one
#define _MERRY_MODE_APPEND_ 2   // if doesn't exist, create one
#define _MERRY_MODE_TRUNCATE_ 3 // if doesn't exist, create one
#define _MERRY_MODE_READ_WRITE_ 4

/*
 Read as binary and write binary are just simply read and write but there are no intermediate conversions done on the data.
*/

// _fd must not be changed
mret_t merry_open_new_file(mstr_t filepath, msize_t mode, mqptr_t _fd);

void merry_close_file(mqptr_t _fd);

msize_t merry_read_from_file(mqptr_t _fd, mbptr_t _store_in, msize_t number_of_bytes);

msize_t merry_write_to_file(mqptr_t _fd, mbptr_t src, msize_t num_of_bytes);

// Seek to _off from relative_pos(or as said in the docs, "whence")
mret_t merry_seek_in_file(mqptr_t _fd, msize_t _off, msize_t relative_pos);

#endif