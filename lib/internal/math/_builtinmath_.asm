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

;; With the current state of the VM, the library strictly requires the support for floating point numbers
;; The assembler also needs modifications to understand numbers such as '1e-123'
;; Writing a math support might just be the extreme for me

dc _MSTD_PI_NUM_ 3.141592653589793238462643383279502884197
dc _MSTD_EULER_NUM_ 2.7182818284590452353602874713526624977572470936999595 ;; The assembler will only use what it can i.e what C++ allows

dc _MSTD_POSITIVE_INF32_ 0b01111111100000000000000000000000
dc _MSTD_NEGATIVE_INF32_ 0b11111111100000000000000000000000

;; Is it just me or is IEEE754 that hard to understand?