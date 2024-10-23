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

;; Standard definitions

dc _MSTD_NULL_ -1

dc _MSTD_FAILURE_ -1
dc _MSTD_SUCCESS_ 0
;; Any other returns may signify something

;; We have to modify this for every platform
;; Platform-specific definitions
dc _M_LINUX_ 1
dc _M_AMD_ 1

dc TRUE 1
dc FALSE 0
dc __x86_64__ 1

dc __LITTLE_ENDIAN 0x00
dc __BIG_ENDIAN 0x01
dc __HYBRID_ENDIAN 0x02

defined __x86_64__
dc _ENDIAN_ [__LITTLE_ENDIAN]
end

defined _M_LINUX_
dc _LLIBS_ 1 ;; use linux libraries
end