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

depends _builtinintr_.asm

proc __builtin_get_errno
proc __builtin_set_errno
proc __builtin_perrno

;; proc __builtin_interpret_errno

;; ARGS: Ma = errno value
__builtin_set_errno
    push Ma
    push Mb
    intr _M_GETERRNO
    storeb Ma, _Mstd_errno
    storeb Mb, _Mstd_errno_vm_  ;; we will interpret ust this
    ret

;; RETURNS: Ma = errno value, Mb = internal error value
__builtin_get_errno
    loadb Ma, _Mstd_errno
    loadb Mb, _Mstd_errno_vm_
    ret

;; ARGS: Ma = PTR to a null-terminated string to be printed before the message
__builtin_perrno

db _Mstd_errno 0
db _Mstd_errno_vm_ 0