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