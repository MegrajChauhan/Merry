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

;; Some quick and useful procedures 

;;proc __builtin_quick_save
;;proc __builtin_quick_restore
proc __builtin_align_value
proc __builtin_exit

;;__builtin_quick_save
;;    pusha
;;    ret

;; __builtin_quick_restore
;;    popa
;;    ret

;; ARG: Ma = The value to align
;; RETURNS: The closest higher multiple of 8 to the value
__builtin_align_value
    push Mb
    add Ma, 7
    movl Mb, 0xFFFFFFFFFFFFFFF8
    and Ma, Mb
    pop Mb
    ret

;; ARGS: Ma = RET value
;; RETURNS: Doesn't return at all
__builtin_exit
    intr _M_EXIT_

depends _builtinintr_.asm