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

dc _M_HALT_ 151               ;; Halts the current core(same behavior as the halt instruction)
dc _M_EXIT_ 152               ;; Kill all threads and exit
dc _M_NEW_CORE_ 153           ;; Add a new vcore 
dc _M_MEM_ 154                ;; Get more memory from the Host OS
dc _M_NEW_PROC_ 155           ;; Start a new procedure
dc _M_BP_ 156                 ;; For debuggers as well(Acts as a breakpoint)
dc _M_LOAD_LIB 157            ;; load a new library
dc _M_UNLOAD_LIB 158          ;; unload a new library
dc _M_GET_AND_CALL_FUNC 159   ;; get a function from a loaded library and call it
dc _M_SYSCALL 160             ;; make a syscall
dc _M_GET_FUNC 161            ;; just get the address of a function from a dynamically loaded library
dc _M_CALL_FUNC 162           ;; use the address got from interrupt 163 to call it as the function 
dc _M_START_SUBSYS 163
dc _M_SUBSYS_ADD_CHANNEL 164
dc _M_SUBSYS_CLOSE_CHANNEL 165
dc _M_SUBSYS_SEND 166
dc _M_SUBSYS_SEND_WAIT 167
dc _M_SUBSYS_STATUS 168
dc _M_GETERRNO 169