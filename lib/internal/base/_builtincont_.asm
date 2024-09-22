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

;; This is the implementation of dynamic containers

depends _builtinalloc_.asm
depends _builtinerrno_.asm
depends _builtinmlocks_.asm

proc __builtin_std_cont_create
proc __builtin_std_cont_push
proc __builtin_std_cont_pop
proc __builtin_std_cont_pfirst ;; pointer to first element
proc __builtin_std_cont_plast  ;; pointer to last element
proc __builtin_std_cont_pat    ;; pointer to the asked element
proc __builtin_std_cont_resize 
proc __builtin_std_cont_erase
proc __builtin_std_cont_eraseat
proc __builtin_std_cont_destroy
proc __builtin_std_cont_is_empty
proc __builtin_std_cont_size    ;; Number of elements 
proc __builtin_std_cont_capacity
proc __builtin_std_cont_elen    ;; per element length
proc __builtin_std_cont_at
proc __builtin_std_cont_append
proc __builtin_std_cont_insert
proc __builtin_std_cont_find
proc __builtin_std_cont_find_last_of
proc __builtin_std_cont_find_from
proc __builtin_std_cont_search       ;; search for a specific element and return its number of appearance
proc __builtin_std_cont_group_search ;; search for a group of elements
proc __builtin_std_cont_is_dynamic
proc __builtin_std_cont_set_dynamic
proc __builtin_std_cont_get_allocator ;; set a custom allocator to be used while creating(It must take the same arguments as the standard functions)
                                      ;; Once an element has been pushed, you may not set an allocator
                                      ;; In fact, once the internal allocator has been used, you may not use your own allocator
                                      ;; This will return a data structure containing the saved addresses
proc __builtin_std_cont_set_find ;; Since the container compares every byte when using __builtin_std_cont_find, some data structures might have a different requirement
                                 ;; Use this to provide the required comparison procedure
proc __builtin_std_cont_set_group_search

proc __builtin_std_cont_def_find_proc
proc __builtin_std_cont_def_grp_search_proc

;; ARGS: Ma = ELEN, Mb = FLAG for dynamic(1 for true else 0), Mc = 0 for custom allocator else anything
;; If Mc == 0 then these arguments are needed too:
;; Md = alloc_proc, M1 = find_proc, M2 = group_search_proc, M3 = realloc_proc, M4 = free_proc   
;; RETURNS: Ma = NULL for failure else a pointer to the new container
__builtin_std_cont_create
    call __builtin_quick_save
    mov Mm2, Ma
    movl Mm1, _MSTD_NULL_
    sss Mm1, 1    

    ;; first get an allocator
    ;; we only use allocator this once and thus we don't save it
    movl Ma, _MSTD_CONT_SIZE_

    cmp Mc, 0
    je _std_cont_create_custom_alloc_setup
    
    call __builtin_std_alloc
    cmp Ma, NULL
    je _std_cont_create_done

    push Ma
    movl M1, __builtin_std_cont_def_find_proc
    add Ma, _MSTD_GET_FIND_PROC_
    storeq M1, Ma ;; find proc
    movl M1, __builtin_std_cont_def_grp_search_proc
    add Ma, 8
    storeq M2, Ma ;; grp_search_proc 
    movl M1, __builtin_std_realloc
    add Ma, 8
    storeq M3, Ma  ;; realloc_proc
    movl M1, __builtin_std_free
    add Ma, 8
    storeq M4, Ma  ;; free_proc
    pop Ma

    jmp _std_cont_create_continue

 _std_cont_create_custom_alloc_setup
    call Md
    cmp Ma, _MSTD_NULL_
    je _std_cont_create_done

    push Ma
    add Ma, _MSTD_GET_FIND_PROC_
    storeq M1, Ma ;; do note that we don't perform argument validation
    add Ma, 8
    storeq M2, Ma ;; grp_search_proc 
    add Ma, 8
    storeq M3, Ma  ;; realloc_proc
    add Ma, 8
    storeq M4, Ma  ;; free_proc
    pop Ma

 _std_cont_create_continue
    mov Mm1, Ma
    ;; we now have the memory we need
    ;; initialize everything else 
    storeb Mb, Ma ;; store the flag

    inc Ma
    call __builtin_std_raw_release ;; this will initialize it
    
    add Ma, 7
    storeq Mm2, Ma ;; elen
    add Ma, 8
    xor Mm3, Mm3
    storeq Mm3, Ma ;; element count
    add Ma, 8
    movl Mm3, 20  ;; start with 20?
    storeq Mm3, Ma ;; capacity
    
    mul Mm2, 20
    mov Ma, Mm2
    call Md    ;; allocate for Array
    cmp Ma, _MSTD_NULL_
    je _std_cont_create_failure ;; we failed
    sss Mm1, 1  ;; this is what we shall return because we succeeded
    add Mm1, _MSTD_GET_ARRAY_
    storeq Ma, Mm1 ;; store the array address
    jmp _std_cont_create_done 
 _std_cont_create_failure
    mov Ma, Mm1
    call M4 ;; free the container
 _std_cont_create_done
    call __builtin_quick_restore
    ret

;; ARGS: Ma = PTR to the container
__builtin_std_cont_destroy
    cmp Ma, _MSTD_NULL_
    je _std_cont_destroy ;; we will be lenient with this
    push Mm1
    mov Mm1, Ma
    add Mm1, _MSTD_GET_LOCK_
    call __builtin_std_raw_release ;; make sure to release it just in case
    

 _std_cont_destroy
    ret

;; The structure of a container
;; BYTE is_dynamic
;; BYTE lock
;; BYTE resb[6]
;; QWORD elen
;; QWORD element_count
;; QWORD element_capacity
;; QWORD find_proc
;; QWORD group_search_proc
;; QWORD realloc_proc
;; QWORD free_proc
;; QWORD *array

dc _MSTD_CONT_SIZE_ 72 ;; A lot of bytes
dc _MSTD_GET_ARRAY_ 64
dc _MSTD_GET_FREE_PROC_ 56
dc _MSTD_GET_REALLOC_PROC_ 48
dc _MSTD_GET_GRP_SEARCH_PROC_ 40
dc _MSTD_GET_FIND_PROC_ 32
dc _MSTD_GET_CAPACITY_ 24
dc _MSTD_GET_COUNT_ 16
dc _MSTD_GET_ELEN_ 8
dc _MSTD_GET_LOCK_ 1
dc _MSTD_GET_DYN_FLAG_ 0

;; Given how the internal allocator requires that the memory isn't meddled with, if you don't want your allocator's data to be overwritten, do not use it.
;; Any stdlib construct will allow you to specify a custom allocator which can do whatever it wants with the entire memory.
;; The stdlib allocator and a custom allocator may work together by letting the custom allocator suppress the stdlib allocator's influence to only the first 
;; pages we get. This, however, is useless since the things depending on the stdlib allocator won't get their requests fulfilled later on during program execution.
;; This means- it's either the stdlib allocator or the custom allocator.
;; This module requires the allocators to be initialized.

