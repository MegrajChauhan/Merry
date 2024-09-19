;; Currently supported errors
dc _M_EACCES 0         ;; Permission denied
dc _M_EEXIST 1         ;; File exists
dc _M_EFAULT 2         ;; Bad address
dc _M_EINTR 3          ;; Interrupted function call
dc _M_EINVAL 4         ;; Invalid argument
dc _M_EIO 5            ;; Input/output error
dc _M_EISDIR 6         ;; Is a directory
dc _M_EMFILE 7         ;; Too many open files
dc _M_ENAMETOOLONG 8   ;; File name too long
dc _M_ENFILE 9         ;; Too many open files in system
dc _M_ENODEV 10        ;; No such device
dc _M_ENOENT 11        ;; No such file or directory
dc _M_ENOMEM 12        ;; Not enough space
dc _M_ENOSPC 13        ;; No space left on device
dc _M_ENOTDIR 14       ;; Not a directory
dc _M_ENXIO 15         ;; No such device or address
dc _M_EOVERFLOW 16     ;; Value too large to be stored in data type
dc _M_EROFS 17         ;; Read-only file system
dc _M_ETIMEDOUT 18     ;; Connection timed out
dc _M_ECONNREFUSED 19  ;; Connection refused
dc _M_EHOSTUNREACH 20  ;; No route to host
dc _M_EADDRINUSE 21    ;; Address already in use
dc _M_EADDRNOTAVAIL 22 ;; Address not available
dc _M_EAFNOSUPPORT 23  ;; Address family not supported
dc _M_ENOTCONN 24      ;; Socket is not connected
dc _M_ECONNRESET 25    ;; Connection reset
dc _M_EPIPE 26         ;; Broken pipe
dc _M_EBADF 27         ;; Bad file descriptor
dc _M_ELOOP 28         ;; Too many levels of symbolic links
dc _M_ENOTEMPTY 29     ;; Directory not empty
dc _M_EPERM 30         ;; Operation not permitted
dc _M_ESHUTDOWN 31     ;; Cannot send after transport endpoint shutdown
dc _M_NODBG 32         ;; Debugging was enabled for the child but the debugger wasn't connected at all
dc _M_SYSCALLERR 33    ;; The syscall being handled by the OS didn't expect the provided argument