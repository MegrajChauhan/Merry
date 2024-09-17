depends _syscalls_.asm

proc std::fio::fopen
proc std::fio::fclose

;; ARGS: Ma = PTR to filename, Mb = Flags, Mc = Mode(for _M_CREATE_ or _M_TMPFILE_)
;; RETURNS: Ma = File Descriptor sent by the OS
;; Note: Doesn't save states
std::fio::fopen
    call __builtin_std_file_open
    ret

;; ARGS: Ma = FD that was given by __builtin_std_file_open
std::fio::fclose
    call __builtin_std_file_close
    ret