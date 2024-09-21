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
;; NOTE: bif = Built-In Features
depends _builtindefs_.asm
depends _builtinintr_.asm
depends _builtinutils_.asm
depends _builtinerrno_.asm
depends _builtinmlocks_.asm

proc __builtin_std_dynl_init
proc __builtin_std_loadlib
proc __builtin_std_unloadlib
proc __builtin_std_symget
proc __builtin_std_symcall
proc __builtin_std_symgc      ;; get and call

__builtin_std_dynl_init
    push Ma
    loadb Ma, _Mstd_dyn_init_ 
    cmp Ma, 1
    je _dyn_initialized

    mov Ma, 1
    storeq Ma, _Mstd_dyn_init_

    mov Ma, 0
    storeq Ma, _Mstd_dyn_loc_

 _dyn_initialized
    pop Ma
    ret

;; ARGS: Ma = PTR to the library name
;; RETURNS: Ma = descriptor else 1 for error 
__builtin_std_loadlib
    call __builtin_quick_save
    push Ma
    mov Ma, _Mstd_dyn_loc_
    call __builtin_std_raw_acquire

    pop Ma
    intr _M_LOAD_LIB
    push Ma
    pop _Mstd_dyn_intermediate_
    cmp Ma, 1
    
    je _dynl_err
    mov Ma, Mb
    jmp _dynl_done

 _dynl_err
    mov Ma, Mb
    call __builtin_set_errno
    movl Ma, 1

 _dynl_done
    call __builtin_quick_restore
    loadq Ma, _Mstd_dyn_intermediate_
    call __builtin_std_raw_release
    ret

;; ARGS: Ma = Handle
__builtin_std_unloadlib
    intr _M_UNLOAD_LIB
    ret

;; ARGS: Ma = handle, Mb = PTR to symbol's name
;; RETURNS: Mb = address else Ma = 1 for error
__builtin_std_symget
    call __builtin_quick_save
    call __builtin_std_raw_acquire
    
    intr _M_GET_FUNC
    push Ma
    pop _Mstd_dyn_intermediate_
    cmp Ma, 0
    jne _symget_err

    jmp _symget_done

 _symget_err
    mov Ma, Mb
    call __builtin_set_errno

 _symget_done
    call __builtin_quick_restore
    loadq Ma, _Mstd_dyn_intermediate_
    call __builtin_std_raw_release
    ret

;; ARGS: Ma = handle, Mb = function address, Mc = parameter, Md = length
;; RETURNS: Ma = function call result else 1 
__builtin_std_symcall
    call __builtin_quick_save
    call __builtin_std_raw_acquire
    
    intr _M_CALL_FUNC
    push Ma
    pop _Mstd_dyn_intermediate_
    cmp Ma, 0
    jne _symcall_err

    jmp _symcall_done
 
 _symcall_err
    mov Ma, Mb
    call __builtin_set_errno

 _symcall_done
    call __builtin_quick_restore
    loadq Ma, _Mstd_dyn_intermediate_
    call __builtin_std_raw_release
    ret

;; ARGS: Ma = handle, Mb = PTR to function name, Mc = parameter, Md = length
;; RETURNS: Ma = function call result else 1 
__builtin_std_symgc
    call __builtin_quick_save
    call __builtin_std_raw_acquire
    
    intr _M_GET_AND_CALL_FUNC
    push Ma
    pop _Mstd_dyn_intermediate_
    cmp Ma, 0
    jne _symgc_err

    jmp _symgc_done

 _symcall_err
    mov Ma, Mb
    call __builtin_set_errno

 _symgc_done
    call __builtin_quick_restore
    loadq Ma, _Mstd_dyn_intermediate_
    call __builtin_std_raw_release
    ret

rb _Mstd_dyn_loc_ 1 ;; This library's lock
                    ;; Only allow one procedure to be performed
                    ;; The VM internally handles if multiple attempts to load the same library are made.
                    ;; We can't have one thread trying to get a symbol while the other unloading the library

rq _Mstd_dyn_intermediate_ 1 ;; intermediate value storage 
rb _Mstd_dyn_init_ 1         ;; Has the module been initialized?

;; NOTE: Symbols have a very specific type. In terms of the C language, the symbol must have the following kind of definition
;; uint64_t name(void* arg, size_t arg_len);
;; The arg_len signifies the number of bytes that 'arg' contains
;; 'arg' is dynamically allocated such that the called function doesn't have the ability
;; to make changes to the data memory.
;; We may need to add a way to handle the allocation efficiently based on 'len'.
;; The return value of the function may be used by the program to do something internally.