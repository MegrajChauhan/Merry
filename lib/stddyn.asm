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

;; This is all thread-safe
;; The libraries are platform-dependent and hence one must compile one library for multiple platforms in order to load them properly

depends _builtindyn_.asm

proc std::dyn::dyn_init
proc std::dyn::load
proc std::dyn::unload
proc std::dyn::getsym
proc std::dyn::callsym
proc std::dyn::gcsym     ;; get and call symbol

;; No args and returns
std::dyn::dyn_init
    call __builtin_std_dynl_init
    ret

;; ARGS: Ma = PTR to the library name
;; RETURNS: Ma = descriptor else 1 for error 
std::dyn::load
    call __builtin_std_loadlib
    ret

;; ARGS: Ma = handle
std::dyn::unload
    call __builtin_std_unloadlib
    ret

;; ARGS: Ma = handle, Mb = PTR to symbol's name
;; RETURNS: Mb = address else Ma = 1 for error
std::dyn::getsym
    call __builtin_std_symget
    ret

;; ARGS: Ma = handle, Mb = function address, Mc = parameter, Md = length
;; RETURNS: Ma = function call result else 1 
std::dyn::callsym
    call __builtin_std_symcall
    ret

;; ARGS: Ma = handle, Mb = PTR to symbol name, Mc = parameter, Md = length
;; RETURNS: Ma = function call result else 1 
std::dyn::gcsym
    call __builtin_std_symgc
    ret