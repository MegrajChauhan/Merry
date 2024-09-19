depends _builtinsyscall_.asm
depends _builtinplatformconstants_.asm
depends _builtindefs_.asm
depends _builtinutils_.asm
depends _builtinerrno_.asm

proc __builtin_std_file_open
proc __builtin_std_file_close
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

;; It will be good to note that file I/O is very complicated.
;; Linux provides more than what this module provides.
;; The complexity of every procedure here would be immense if we were
;; to try to make it safe and encompass everything.