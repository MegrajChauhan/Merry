depends _syscalls_.asm

proc std::fio::fopen
proc std::fio::fclose
proc std::fio::fwrite
proc std::fio::fread

;; ARGS: Ma = PTR to filename, Mb = Flags, Mc = Mode(for _M_CREATE_ or _M_TMPFILE_)
;; RETURNS: Ma = File Descriptor sent by the OS else -1
;; Note: Doesn't save states
std::fio::fopen
    call __builtin_std_file_open
    ret

;; ARGS: Ma = FD that was given by __builtin_std_file_open
std::fio::fclose
    call __builtin_std_file_close
    ret

;; ARGS: Ma = FD, Mb = PTR to buffer, Mc = number of bytes to read
;; RETURNS: Ma = Number of bytes, 0 or -1 for error 
std::fio::fwrite
    call __builtin_std_file_write
    ret

;; ARGS: Ma = FD, Mb = PTR to buffer, Mc = number of bytes to read
;; RETURNS: Ma = Number of bytes, 0 ==EOF or -1 for error 
std::fio::fread
    call __builtin_std_file_read
    ret