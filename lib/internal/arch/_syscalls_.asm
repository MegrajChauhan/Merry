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

depends _builtinsyscall_.asm
depends _builtinplatformconstants_.asm
depends _builtindefs_.asm
depends _builtinutils_.asm
depends _builtinerrno_.asm

proc __builtin_std_file_open
proc __builtin_std_file_close
proc __builtin_std_file_read
proc __builtin_std_file_write
proc __builtin_std_syscall_check_status

;; ARGS: Ma = PTR to filename, Mb = Flags, Mc = Mode(for _M_CREATE_ or _M_TMPFILE_)
;; RETURNS: Ma = File Descriptor sent by the OS else -1 
;; NOTE: Doesn't save states
__builtin_std_file_open
defined _M_LINUX_
    mov M3, Mc
    mov M2, Mb
    mov M1, Ma
    movl Ma, _M_SYS_OPEN_
    syscall
end
    call __builtin_std_syscall_check_status
    ret

;; ARGS: Ma = FD that was given by __builtin_std_file_open
__builtin_std_file_close
    push Mb
defined _M_LINUX_
    mov Mb, Ma
    movl Ma, _M_SYS_CLOSE_
    syscall
    call __builtin_std_syscall_check_status
end
    pop Mb
    ret

__builtin_std_syscall_check_status
defined _M_LINUX_
    cmp Ma, 0xFFFFFFFFFFFFFFFF
end
    je _std_syscall_check_status_set_errno
    ret

 _std_syscall_check_status_set_errno
    push Ma
    mov Ma, Mb
    call __builtin_set_errno
    pop Ma
    ret

;; ARGS: Ma = FD, Mb = PTR to buffer, Mc = number of bytes to read
;; RETURNS: Ma = Number of bytes, 0 ==EOF or -1 for error 
__builtin_std_file_read
    call __builtin_quick_save
    mov M1, Ma
    mov M2, Mb
    mov M3, Mc
    mov Ma, _M_SYS_READ_
    syscall
    call __builtin_std_syscall_check_status
    call __builtin_quick_restore
    ret

;; ARGS: Ma = FD, Mb = PTR to buffer, Mc = number of bytes to write from buffer
;; RETURNS: Ma = Number of bytes, 0 or -1 for error
__builtin_std_file_write
    call __builtin_quick_save
    mov M1, Ma
    mov M2, Mb
    mov M3, Mc
    mov Ma, _M_SYS_WRITE_
    syscall
    call __builtin_std_syscall_check_status
    call __builtin_quick_restore
    ret

;; It will be good to note that file I/O is very complicated.
;; Linux provides more than what this module provides.
;; The complexity of every procedure here would be immense if we were
;; to try to make it safe and encompass everything.

;;-----------------------------------------------------------------------------------
;; In Linux, everything is a file and hence mostly anything works with a file descriptor.
;; That is not the case in other systems, for eg: Windows and hence only if both platforms support the use of file descriptors
;; or values returned from the use of 'open' syscall, then and only then will we implement it.