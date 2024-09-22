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
;; This module is extremely forgiving.
;; There are no errno set and no terminations as well.
;; Do make use of the return values

;; We also do not use any sort of Magic Number to identify that the pointer
;; passed is in fact a container.
;; This module is too risky to be used in programmer's language.
;; Those who actually use it properly should find it a great helper

depends _builtinalloc_.asm
depends _builtinerrno_.asm
depends _builtinmlocks_.asm

proc __builtin_std_cont_create
proc __builtin_std_cont_destroy
proc __builtin_std_cont_resize 
proc __builtin_std_cont_pfirst  ;; pointer to first element
proc __builtin_std_cont_plast   ;; pointer to last element
proc __builtin_std_cont_pat     ;; pointer to the asked element
proc __builtin_std_cont_at      ;; exactly the same as 'pat' so why is this needed? I just said 'I want it.'
proc __builtin_std_cont_size    ;; get the number of elements 
proc __builtin_std_cont_is_empty
proc __builtin_std_cont_capacity
proc __builtin_std_cont_elen    ;; get per element length
proc __builtin_std_cont_push
proc __builtin_std_cont_pop
proc __builtin_std_cont_erase
proc __builtin_std_cont_eraseat
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
    call __builtin_std_raw_release ;; this will initialize the lock
    
    inc Ma
    storeb Mb, Ma ;; store the flag

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
;; RETURNS: Nothing
;; NOTE: Make sure that every single thread has done what it needs before doing this
__builtin_std_cont_destroy
    call __builtin_quick_save
    cmp Ma, _MSTD_NULL_
    je _std_cont_destroy ;; we will be lenient with this
    mov Mm1, Ma
    mov Mm2, Ma
    call __builtin_std_raw_release ;; make sure to release it just in case
    
    add Mm1, 65 ;; to get to the free proc
    loadq Mm1, Mm1
    add Mm2, _MSTD_GET_ARRAY_
    loadq Mm2, Mm2
    excgq Mm2, Ma
    call Mm1  ;; free the array
    excgq Mm2, Ma ;; get the original container structure
    call Mm1 ;; free the container

 _std_cont_destroy
    call __builtin_quick_restore
    ret

;; ARGS: Ma = PTR to the container, Mb = Factor
;; RETURNS: Ma = PTR to the resized container else NULL
;; NOTE: The container is resized by a factor of 2 each time
;; After each resize, the pointers that you may have for the elements will
;; be invalidated and hence you should not access them
;; THREAD-SAFE
__builtin_std_cont_resize
   call __builtin_quick_save
   call __builtin_std_raw_acquire ;; Ma is pointing to the lock
 
   movl Mm1, _MSTD_NULL_
   sss Mm1, 1

   cmp Ma, _MSTD_NULL_
   je _std_cont_resize_done
   
   mov Mf, Mb
   mov Mm1, Ma
   add Mm1, _MSTD_GET_ELEN_
   loadq Mb, Mm1 ;; get elen
   add Mm1, 16 ;; get the capacity
   loadq Mc, Mm1
   mul Mb, Mf
   storeq Mb, Mm1 ;; save it
   mul Mb, Mc ;; total length
   add Mb, _MSTD_CONT_SIZE_ ;; we must save the container structure's size too
   add Mm1, 24 ;; get the reallocation procedure
   loadq Mm1, Mm1
   call Mm1 ;; Ma hasn't been meddled with

   cmp Ma, _MSTD_NULL_
   je _std_cont_resize_done

   ;; the reallocation must also copy the old contents to the new memory region
   ;; update the internals
   sss Ma, 1 ;; this is what we return 

 _std_cont_resize_done
   call __builtin_std_raw_release
   call __builtin_quick_restore
   ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = PTR to the first element else NULL for no elements pushed
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
__builtin_std_cont_pfirst
   call __builtin_quick_save
   movl Mm1, _MSTD_NULL_
   sss Mm1, 1

   cmp Ma, _MSTD_NULL_
   je _std_cont_pfirst_done

   add Ma, _MSTD_GET_COUNT_
   loadq Mb, Ma

   cmp Mb, 0
   je _std_cont_pfirst_done

   add Ma, 48 ;; to get the array
   loadq Ma, Ma
   sss Ma, 1 ;; this is what we return

 _std_cont_pfirst_done
   call __builtin_quick_restore
   ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = PTR to the last element else NULL for no elements pushed
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
__builtin_std_cont_plast
   call __builtin_quick_save
   movl Mm1, _MSTD_NULL_
   sss Mm1, 1

   cmp Ma, _MSTD_NULL_
   je _std_cont_plast_done

   add Ma, _MSTD_GET_ELEN_
   loadq Mb, Ma
   add Ma, 8
   loadq Mc, Ma
   cmp Mc, 0
   je _std_cont_plast_done
   dec Mc ;; index starts from 0
   mul Mb, Mc

   cmp Mb, 0
   je _std_cont_plast_done

   add Ma, 48 ;; to get the array
   loadq Ma, Ma ;; This gives us the pointer
   add Ma, Mb
   sss Ma, 1 ;; this is what we return

 _std_cont_plast_done
   call __builtin_quick_restore
   ret

;; ARGS: Ma = PTR to a container, Mb = index
;; RETURNS: Ma = PTR to the asked element else NULL if no such element exists
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
__builtin_std_cont_pat
   call __builtin_quick_save
   movl Mm1, _MSTD_NULL_
   sss Mm1, 1

   cmp Ma, _MSTD_NULL_
   je _std_cont_pat_done

   add Ma, _MSTD_GET_ELEN_
   loadq Mc, Ma
   add Ma, 8
   loadq Md, Ma
   dec Md ;; index starts from 0

   cmp Md, -1
   js _std_cont_pat_done
   cmp Md, Mb
   js _std_cont_pat_done

   mul Mb, Ma ;; we will get the required address
   add Ma, 48 ;; to get the array
   loadq Ma, Ma ;; This gives us the pointer
   add Ma, Mb
   sss Ma, 1 ;; this is what we return

 _std_cont_pat_done
   call __builtin_quick_restore
   ret

;; ARGS: Ma = PTR to a container, Mb = index
;; RETURNS: Ma = PTR to the asked element else NULL if no such element exists
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
__builtin_std_cont_at
   call __builtin_std_cont_pat
   ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = The number of elements else -1
;; NOTE: Not thread-safe and hence resizes may affect
__builtin_std_cont_size
   call __builtin_quick_save
   movl Mb, -1
   sss Mb, 1

   cmp Ma, _MSTD_NULL_
   je _std_cont_size_done

   add Ma, _MSTD_GET_COUNT_
   loadq Ma, Ma
   sss Ma, 1 ;; we return this

 _std_cont_size_done
   call __builtin_quick_restore
   ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 1 for false, NULL for error else 0
;; NOTE: Not thread-safe and hence resizes may affect
__builtin_std_cont_is_empty
   call __builtin_quick_save
   movl Mb, _MSTD_NULL_
   sss Mb, 1

   cmp Ma, _MSTD_NULL_
   je _std_cont_is_empty_done

   add Ma, _MSTD_GET_COUNT_
   loadq Ma, Ma
   cmp Ma, 0
   je _std_cont_is_empty_empty

   movl Ma, 1
   sss Ma, 1
   jmp _std_cont_is_empty_done

 _std_cont_is_empty_empty
   movl Ma, 0
   sss Ma, 1

 _std_cont_is_empty_done
   call __builtin_quick_restore
   ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 0 for error else the capacity
;; NOTE: Not thread-safe and hence resizes may affect
__builtin_std_cont_capacity
   call __builtin_quick_save
   movl Mb, 0
   sss Mb, 1

   cmp Ma, _MSTD_NULL_
   je _std_cont_cap_done

   add Ma, _MSTD_GET_CAPACITY_
   loadq Ma, Ma
   sss Ma, 1

 _std_cont_cap_done
   call __builtin_quick_restore
   ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 0 for error else the elen value
;; NOTE: Not thread-safe and hence resizes may affect
__builtin_std_cont_elen
   call __builtin_quick_save
   movl Mb, 0
   sss Mb, 1

   cmp Ma, _MSTD_NULL_
   je _std_cont_elen_done

   add Ma, _MSTD_GET_ELEN_
   loadq Ma, Ma
   sss Ma, 1

 _std_cont_elen_done
   call __builtin_quick_restore
   ret

;; The structure of a container
;; BYTE lock
;; BYTE is_dynamic
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
dc _MSTD_GET_DYN_FLAG_ 1
dc _MSTD_GET_LOCK_ 0

;; Given how the internal allocator requires that the memory isn't meddled with, if you don't want your allocator's data to be overwritten, do not use it.
;; Any stdlib construct will allow you to specify a custom allocator which can do whatever it wants with the entire memory.
;; The stdlib allocator and a custom allocator may work together by letting the custom allocator suppress the stdlib allocator's influence to only the first 
;; pages we get. This, however, is useless since the things depending on the stdlib allocator won't get their requests fulfilled later on during program execution.
;; This means- it's either the stdlib allocator or the custom allocator.
;; This module requires the allocators to be initialized.

