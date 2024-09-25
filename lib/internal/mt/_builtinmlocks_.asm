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

depends _builtindefs_.asm

proc __builtin_std_raw_acquire
proc __builtin_std_raw_release

;; ARGS: Ma = PTR to the mutex lock[Must be a byte]
;; RETURNS: Nothing
;; NOTE: This doesn't save states so save it yourself
__builtin_std_raw_acquire
    movl M1, _MSTD_RELEASE_         ;; expected
    movl M2, _MSTD_ACQUIRE_         ;; desired
    cmpxchg M1, M2, Ma              ;; I apologize for the extremely weird syntax[Please look at asm/tests/masm/test.masm line 228 for details] 
    jne __builtin_std_raw_acquire   ;; keep trying until unlocked
    ret

;; ARGS: Ma = PTR to the mutex lock
;; NOTE: This doesn't save states so save it yourself
__builtin_std_raw_release
    xor Mb, Mb
    atm storeb Mb, Ma
    ret

dc _MSTD_ACQUIRE_ 1
dc _MSTD_RELEASE_ 0