#include "merry_errno.h"

_MERRY_ALWAYS_INLINE_ msize_t merry_get_errno()
{
    msize_t ret = merrno;
    merrno = 0;
    return ret;
}

void merry_update_errno()
{
    switch(errno)
    {
    case EACCES:
        merrno = MERRY_EACCES;
        break;
    case EEXIST:
        merrno = MERRY_EEXIST;
        break;
    case EFAULT:
        merrno = MERRY_EFAULT;
        break;
    case EINTR:
        merrno = MERRY_EINTR;
        break;
    case EINVAL:
        merrno = MERRY_EINVAL;
        break;
    case EIO:
        merrno = MERRY_EIO;
        break;
    case EISDIR:
        merrno = MERRY_EISDIR;
        break;
    case EMFILE:
        merrno = MERRY_EMFILE;
        break;
    case ENAMETOOLONG:
        merrno = MERRY_ENAMETOOLONG;
        break;
    case ENFILE:
        merrno = MERRY_ENFILE;
        break;
    case ENODEV:
        merrno = MERRY_ENODEV;
        break;
    case ENOENT:
        merrno = MERRY_ENOENT;
        break;
    case ENOMEM:
        merrno = MERRY_ENOMEM;
        break;
    case ENOSPC:
        merrno = MERRY_ENOSPC;
        break;
    case ENOTDIR:
        merrno = MERRY_ENOTDIR;
        break;
    case ENXIO:
        merrno = MERRY_ENXIO;
        break;
    case EOVERFLOW:
        merrno = MERRY_EOVERFLOW;
        break;
    case EROFS:
        merrno = MERRY_EROFS;
        break;
    case ETIMEDOUT:
        merrno = MERRY_ETIMEDOUT;
        break;
    case ECONNREFUSED:
        merrno = MERRY_ECONNREFUSED;
        break;
    case EHOSTUNREACH:
        merrno = MERRY_EHOSTUNREACH;
        break;
    case EADDRINUSE:
        merrno = MERRY_EADDRINUSE;
        break;
    case EADDRNOTAVAIL:
        merrno = MERRY_EADDRNOTAVAIL;
        break;
    case EAFNOSUPPORT:
        merrno = MERRY_EAFNOSUPPORT;
        break;
    case ENOTCONN:
        merrno = MERRY_ENOTCONN;
        break;
    case ECONNRESET:
        merrno = MERRY_ECONNRESET;
        break;
    case EPIPE:
        merrno = MERRY_EPIPE;
        break;
    case EBADF:
        merrno = MERRY_EBADF;
        break;
    case ELOOP:
        merrno = MERRY_ELOOP;
        break;
    case ENOTEMPTY:
        merrno = MERRY_ENOTEMPTY;
        break;
    case EPERM:
        merrno = MERRY_EPERM;
        break;
    case ESHUTDOWN:
        merrno = MERRY_ESHUTDOWN;
        break;
    default:
        merrno = -1; // Undefined errno
        break;
    }
}

// Function to interpret custom merrno values and log appropriate messages
void merry_interpret_errno()
{
    // The messages print here will leave anyone in confusion
    // What the hell?! What caused this error?!
    // To solve this problem, the component that calls this function will display extra information for context
    switch (merrno)
    {
    case MERRY_EACCES:
        inerr_log("Permission denied.");
        break;
    case MERRY_EEXIST:
        inerr_log("File exists.");
        break;
    case MERRY_EFAULT:
        inerr_log("Bad address.");
        break;
    case MERRY_EINTR:
        inerr_log("Interrupted function call.");
        break;
    case MERRY_EINVAL:
        inerr_log("Invalid argument.");
        break;
    case MERRY_EIO:
        inerr_log("Input/output error.");
        break;
    case MERRY_EISDIR:
        inerr_log("Is a directory.");
        break;
    case MERRY_EMFILE:
        inerr_log("Too many open files.");
        break;
    case MERRY_ENAMETOOLONG:
        inerr_log("File name too long.");
        break;
    case MERRY_ENFILE:
        inerr_log("Too many open files in system.");
        break;
    case MERRY_ENODEV:
        inerr_log("No such device.");
        break;
    case MERRY_ENOENT:
        inerr_log("No such file or directory.");
        break;
    case MERRY_ENOMEM:
        inerr_log("Not enough space.");
        break;
    case MERRY_ENOSPC:
        inerr_log("No space left on device.");
        break;
    case MERRY_ENOTDIR:
        inerr_log("Not a directory.");
        break;
    case MERRY_ENXIO:
        inerr_log("No such device or address.");
        break;
    case MERRY_EOVERFLOW:
        inerr_log("Value too large to be stored in data type.");
        break;
    case MERRY_EROFS:
        inerr_log("Read-only file system.");
        break;
    case MERRY_ETIMEDOUT:
        inerr_log("Connection timed out.");
        break;
    case MERRY_ECONNREFUSED:
        inerr_log("Connection refused.");
        break;
    case MERRY_EHOSTUNREACH:
        inerr_log("No route to host.");
        break;
    case MERRY_EADDRINUSE:
        inerr_log("Address already in use.");
        break;
    case MERRY_EADDRNOTAVAIL:
        inerr_log("Address not available.");
        break;
    case MERRY_EAFNOSUPPORT:
        inerr_log("Address family not supported.");
        break;
    case MERRY_ENOTCONN:
        inerr_log("Socket is not connected.");
        break;
    case MERRY_ECONNRESET:
        inerr_log("Connection reset.");
        break;
    case MERRY_EPIPE:
        inerr_log("Broken pipe.");
        break;
    case MERRY_EBADF:
        inerr_log("Bad file descriptor.");
        break;
    case MERRY_ELOOP:
        inerr_log("Too many levels of symbolic links.");
        break;
    case MERRY_ENOTEMPTY:
        inerr_log("Directory not empty.");
        break;
    case MERRY_EPERM:
        inerr_log("Operation not permitted.");
        break;
    case MERRY_ESHUTDOWN:
        inerr_log("Cannot send after transport endpoint shutdown.");
        break;
    default:
        inerr_log("Unknown error.");
        break;
    }
    merrno = 0; // Reset merrno after logging the error
}

void merry_set_errno(msize_t _err)
{
    merrno = _err;
}