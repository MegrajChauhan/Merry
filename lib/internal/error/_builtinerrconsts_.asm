;; MIT License
;;
;; Copyright (c) 2024 MegrajChauhan
;;
;; Permission is hereby granted, free of charge, to any person obtaining a copy
;; of this software and associated documentation files (the "Software"), to deal
;; in the Software without restriction, including without limitation the rights
;; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;; copies of the Software, and to permit persons to whom the Software is
;; furnished to do so, subject to the following conditions:
;;
;; The above copyright notice and this permission notice shall be included in all
;; copies or substantial portions of the Software.
;;
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;; SOFTWARE.

;; Currently supported errors
dc _M_NODBG 0       ;; Debugging was enabled for the child but the debugger wasn't connected at all
dc _M_SYSCALLERR 1  ;; The syscall being handled by the OS didn't expect the provided argument
dc _M_DYNERR 2      ;; The recent dynamic library related request failed
dc _M_DYNCLOSED 3   ;; The handle has already been closed
dc _M_INIT_FAILED 4 ;; Failed to initialize something
dc _M_INVAL_ARG 5   ;; invalid argument
dc _M_SYSERR 6      ;; error that was caused by the host system itself
dc _M_VMERR 7       ;; the error was due to the VM

;; Each operating system has its own set of ERRNO constants with many common
;; ones as well. The values may differ between the platforms and hence we
;; will need to handle that inside the stdlib itself.
;; dc _M_EPERM 8       ;; Operation not permitted
;; dc _M_ENOENT 9      ;; No such file or directory
;; dc _M_ESRCH 10      ;; No such process
;; dc _M_EINTR 11      ;; Interrupted system call
;; dc _M_EIO 12        ;; I/O error
;; dc _M_ENXIO 13      ;; No such device or address
;; dc _M_E2BIG 14      ;; Argument list too long
;; dc _M_ENOEXEC 15    ;; Exec format error
;; dc _M_EBADF 16      ;; Bad file number
;; dc _M_ECHILD 17     ;; No child processes
;; dc _M_EAGAIN 18     ;; Try again
;; dc _M_ENOMEM 19     ;; Out of memory
;; dc _M_EACCES 20     ;; Permission denied
;; dc _M_EFAULT 21     ;; Bad address
;; dc _M_ENOTBLK 22    ;; Block device required
;; dc _M_EBUSY 23      ;; Device or resource busy
;; dc _M_EEXIST 24     ;; File exists
;; dc _M_EXDEV 25      ;; Cross-device link
;; dc _M_ENODEV 26     ;; No such device
;; dc _M_ENOTDIR 27    ;; Not a directory
;; dc _M_EISDIR 28     ;; Is a directory
;; dc _M_EINVAL 29     ;; Invalid argument
;; dc _M_ENFILE 30     ;; File table overflow
;; dc _M_EMFILE 31     ;; Too many open files
;; dc _M_ENOTTY 32     ;; Not a typewriter
;; dc _M_ETXTBSY 33    ;; Text file busy
;; dc _M_EFBIG 34      ;; File too large
;; dc _M_ENOSPC 35     ;; No space left on device
;; dc _M_ESPIPE 36     ;; Illegal seek
;; dc _M_EROFS 37      ;; Read-only file system
;; dc _M_EMLINK 38     ;; Too many links
;; dc _M_EPIPE 39      ;; Broken pipe