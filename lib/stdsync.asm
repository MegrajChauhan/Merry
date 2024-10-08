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

depends _builtinmlocks_.asm
depends _builtinmcvars_.asm

proc std::sync::mutex_init
proc std::sync::mutex_lock
proc std::sync::mutex_unlock
proc std::sync::condition_init
proc std::sync::condition_wait
proc std::sync::condition_signal
proc std::sync::condition_broadcast

;; ARGS: Ma = PTR to a byte variable
;; RETURNS: Nothing but initializes the lock
;; NOTE: Doesn't save states
std::sync::mutex_init
    movl Mb, _MSTD_RELEASE_   ;; start with the released state
    storeb Mb, Ma
    ret

;; ARGS: Ma = PTR to an initialized lock
;; RETURNS: Nothing
;; NOTE: Doesn't save states
std::sync::mutex_lock
    call __builtin_std_raw_acquire
    ret

;; ARGS: Ma = PTR to an initialized lock
;; RETURNS: Nothing
;; NOTE: Doesn't save states
std::sync::mutex_unlock
    call __builtin_std_raw_release
    ret

;; ARGS: Ma = PTR to a variable of type WORD i.e 2 bytes long
;; RETURNS: Nothing but initializes a new condition variable 
;; NOTE: Doesn't save states
std::sync::condition_init
    xor Mb, Mb
    storew Mb, Ma
    ret

;; ARGS: Ma = PTR to an initialized condition variable, Mb = PTR to an initialized mutex lock
;; RETURNS: Nothing
;; NOTE: Doesn't save states
std::sync::condition_wait
    call __builtin_std_raw_condition_variable_wait
    ret

;; ARGS: Ma = PTR to an initialized condition variable
;; RETURNS: Nothing
;; NOTE: Doesn't save states
std::sync::condition_signal
    call __builtin_std_raw_condition_variable_signal
    ret

;; ARGS: Ma = PTR to an initialized condition variable
;; RETURNS: Nothing
;; NOTE: Doesn't save states
std::sync::condition_broadcast
    call __builtin_std_raw_condition_variable_broadcast
    ret