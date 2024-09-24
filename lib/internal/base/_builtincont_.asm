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

depends _builtinalloc_.asm
depends _builtinerrno_.asm
depends _builtinmlocks_.asm

proc __builtin_std_cont_create
proc __builtin_std_cont_destroy
proc __builtin_std_cont_resize 
proc __builtin_std_cont_capinc 
proc __builtin_std_cont_pfirst  ;; pointer to first element
proc __builtin_std_cont_plast   ;; pointer to last element
proc __builtin_std_cont_pat     ;; pointer to the asked element
proc __builtin_std_cont_at      ;; exactly the same as 'pat' so why is this needed? I just said 'I want it.'
proc __builtin_std_cont_size    ;; get the number of elements 
proc __builtin_std_cont_is_empty
proc __builtin_std_cont_is_full
proc __builtin_std_cont_capacity
proc __builtin_std_cont_elen    ;; get per element length
proc __builtin_std_cont_erase
proc __builtin_std_cont_is_dynamic
proc __builtin_std_cont_set_dynamic
proc __builtin_std_cont_get_allocator ;; set a custom allocator to be used while creating(It must take the same arguments as the standard functions)
                                      ;; Once an element has been pushed, you may not set an allocator
                                      ;; In fact, once the internal allocator has been used, you may not use your own allocator
                                      ;; This will return a data structure containing the saved addresses
proc __builtin_std_cont_set_find ;; Since the container compares every byte when using __builtin_std_cont_find, some data structures might have a different requirement
                                 ;; Use this to provide the required comparison procedure
proc __builtin_std_cont_set_group_search
proc __builtin_std_cont_push
proc __builtin_std_cont_pop
;; proc __builtin_std_cont_shiftl
;; proc __builtin_std_cont_shiftr
proc __builtin_std_cont_eraseat
proc __builtin_std_cont_append
proc __builtin_std_cont_insert
proc __builtin_std_cont_find
proc __builtin_std_cont_find_last_of
proc __builtin_std_cont_find_from
proc __builtin_std_cont_search       ;; search for a specific element and return its number of appearance
proc __builtin_std_cont_group_search ;; search for a group of elements

proc __builtin_std_cont_def_find_proc
proc __builtin_std_cont_def_grp_search_proc

;; ARGS: Ma = ELEN, Mb = FLAG for dynamic(1 for true else 0), Mc = 0 for custom allocator else anything
;; If Mc == 0 then these arguments are needed too:
;; Md = alloc_proc, M1 = find_proc, M2 = group_search_proc, M3 = realloc_proc, M4 = free_proc   
;; RETURNS: Ma = NULL for failure else a pointer to the new container(Sets ERRNO)
__builtin_std_cont_create
  push 0
  call __builtin_quick_save
  movl Mm1, _MSTD_NULL_
  sss Mm1, 1   ;; Return value for error

  ;; First we need to allocate
  cmp Ma, 0
  je _std_cont_create_inval_args
  cmp Mc, 0
  je _std_cont_create_custom_procs

  ;; The default procedures are to be used
  movl Md, __builtin_std_alloc
  movl M1, __builtin_std_cont_def_find_proc
  movl M2, __builtin_std_cont_def_grp_search_proc
  movl M3, __builtin_std_realloc
  movl M4, __builtin_std_free

  jmp _std_cont_create_alloc
 _std_cont_create_custom_procs
  ;; Custom procedures have been provided
  cmp Md, _MSTD_NULL_
  cmp M1, _MSTD_NULL_
  cmp M2, _MSTD_NULL_
  cmp M3, _MSTD_NULL_
  cmp M4, _MSTD_NULL_
  je _std_cont_create_inval_args

 _std_cont_create_alloc
  ;; allocate for the container first
  mov Mf, Ma ;; temporary(Elen)
  movl Ma, _MSTD_CONT_SIZE_
  call Md
  cmp Ma, _MSTD_NULL_
  je _std_cont_create_done ;; we failed!

  ;; we now have a container
  xor Mm1, Mm1
  mov Mm2, Ma ;; temporary(allocated container)
  storeb Mm1, Ma ;; LOCK
  add Ma, 1
  storeb Mb, Ma  ;; DYNAMIC FLAG
  add Ma, 7
  storeq Mf, Ma  ;; ELEN
  add Ma, 8
  storeq Mm1, Ma ;; COUNT
  add Ma, 8
  mov Me, 20     ;; starting capacity
  storeq Me, Ma  ;; CAPACITY
  add Ma, 8
  storeq M1, Ma  ;; FIND PROC
  add Ma, 8
  storeq M2, Ma  ;; GRP SEARCH PROC
  add Ma, 8
  storeq M3, Ma  ;; REALLOC PROC
  add Ma, 8
  storeq M4, Ma  ;; FREE PROC
  add Ma, 8      
  mov Mm1, Ma    ;; temporary(Array)
  mov Ma, 20
  mul Ma, Mf  ;; total size
  call Md     ;; allocate array
  cmp Ma, _MSTD_NULL_
  je _std_cont_create_failed
  
  storeq Ma, Mm1
  sss Mm2, 1
  jmp _std_cont_create_done

 _std_cont_create_failed
  mov Ma, Mm2 ;; get the container
  call M4     ;; free proc 

 _std_cont_create_inval_args
  movl Ma, _M_EINVAL
  call __builtin_set_errno

 _std_cont_create_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to the container
;; RETURNS: Nothing
;; NOTE: Make sure that every single thread has done what it needs before doing this
__builtin_std_cont_destroy
  call __builtin_quick_save
  cmp Ma, _MSTD_NULL_
  je _std_cont_destroy_done

  push Ma
  add Ma, _MSTD_GET_FREE_PROC_
  loadq Mb, Ma ;; free proc
  add Ma, 8
  loadq Ma, Ma ;; array
  call Mb ;; free the array
  pop Ma
  call Mb ;; free the container

 _std_cont_destroy_done
  call __builtin_quick_restore
  ret

;; ARGS: Ma = PTR to the container, Mb = Factor
;; RETURNS: Ma = PTR to the container else NULL
;; NOTE: The container is resized by a factor of FACTOR
;; After each resize, the pointers that you may have for the elements will
;; be invalidated and hence you should not access them
;; THREAD-SAFE
__builtin_std_cont_resize
  push _MSTD_NULL_
  call __builtin_quick_save
  cmp Ma, _MSTD_NULL_
  cmp Mb, 0
  cmp Mb, 1 ;; this is also invalid
  je _std_cont_resize_inval
    
  call __builtin_std_raw_acquire
  sss Ma, 1
  ;; get all the data first
  inc Ma
  loadb Mc, Ma
  cmp Mc, 1
  jne _std_cont_resize_failed
  add Ma, 7
  loadq M1, Ma ;; ELEN
  add Ma, 8
  loadq M2, Ma ;; COUNT
  add Ma, 8
  loadq M3, Ma ;; CAPACITY
  add Ma, 32
  loadq M4, Ma ;; realloc proc
  add Ma, 16
  loadq Ma, Ma ;; the array
  
  ;; do calculations
  mul Mb, M3
  push Mb
  mul Mb, M1 ;; new total number of bytes

  ;; re-allocate
  call M4 
  pop Mb
  cmp Ma, _MSTD_NULL_
  je _std_cont_resize_failed

  ;; update internals
  mov Mc, Ma
  gss Ma, 1
  add Ma, _MSTD_GET_CAPACITY_
  storeq Mb, Ma
  add Ma, 40
  storeq Mc, Ma
  
 _std_cont_resize_failed ;; Doesn't mean we truly failed
  gss Ma, 1
  call __builtin_std_raw_release
  jmp _std_cont_resize_done

 _std_cont_resize_inval
  movl Ma, _M_EINVAL
  call __builtin_set_errno

 _std_cont_resize_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to the container, Mb = Capacity to add
;; RETURNS: Ma = PTR to the container else NULL
;; NOTE: The container is increased by Mb
;; After this, the pointers that you may have for the elements will
;; be invalidated and hence you should not access them
;; THREAD-SAFE
__builtin_std_cont_capinc
  push _MSTD_NULL_
  call __builtin_quick_save
  cmp Ma, _MSTD_NULL_
  cmp Mb, 0
  je _std_cont_resize_inval
    
  call __builtin_std_raw_acquire
  sss Ma, 1
  ;; get all the data first
  inc Ma
  loadb Mc, Ma
  cmp Mc, 1
  jne _std_cont_resize_failed
  add Ma, 7
  loadq M1, Ma ;; ELEN
  add Ma, 8
  loadq M2, Ma ;; COUNT
  add Ma, 8
  loadq M3, Ma ;; CAPACITY
  add Ma, 32
  loadq M4, Ma ;; realloc proc
  add Ma, 16
  loadq Ma, Ma ;; the array
  
  ;; do calculations
  add Mb, M3
  push Mb
  mul Mb, M1 ;; new total number of bytes

  ;; re-allocate
  call M4 
  pop Mb
  cmp Ma, _MSTD_NULL_
  je _std_cont_resize_failed

  ;; update internals
  mov Mc, Ma
  gss Ma, 1
  add Ma, _MSTD_GET_CAPACITY_
  storeq Mb, Ma
  add Ma, 40
  storeq Mc, Ma
  
 _std_cont_resize_failed ;; Doesn't mean we truly failed
  gss Ma, 1
  call __builtin_std_raw_release
  jmp _std_cont_resize_done

 _std_cont_resize_inval
  movl Ma, _M_EINVAL
  call __builtin_set_errno

 _std_cont_resize_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = PTR to the first element else NULL for no elements pushed
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
__builtin_std_cont_pfirst
  push _MSTD_NULL_
  call __builtin_quick_save

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
  pop Ma
  ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = PTR to the last element else NULL for no elements pushed
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
__builtin_std_cont_plast
  push _MSTD_NULL_
  call __builtin_quick_save

  cmp Ma, _MSTD_NULL_
  je _std_cont_plast_done

  add Ma, _MSTD_GET_ELEN_
  loadq Mb, Ma
  add Ma, 8
  loadq Mc, Ma ;;Count
  cmp Mc, 0
  je _std_cont_plast_done
  dec Mc ;; index starts from 0
  mul Mb, Mc

  add Ma, 48   ;; to get the array
  loadq Ma, Ma ;; This gives us the pointer
  add Ma, Mb
  sss Ma, 1    ;; this is what we return

 _std_cont_plast_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to a container, Mb = index
;; RETURNS: Ma = PTR to the asked element else NULL if no such element exists
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
__builtin_std_cont_pat
  push _MSTD_NULL_
  call __builtin_quick_save

  cmp Ma, _MSTD_NULL_
  je _std_cont_pat_done

  add Ma, _MSTD_GET_ELEN_
  loadq Mc, Ma
  add Ma, 8
  loadq Md, Ma ;; count
  cmp Md, 0
  js _std_cont_pat_done
  dec Md       ;; index starts from 0
  cmp Md, Mb
  js _std_cont_pat_done
  mul Mb, Ma   ;; we will get the required address
  add Ma, 48   ;; to get the array
  loadq Ma, Ma ;; This gives us the pointer
  add Ma, Mb
  sss Ma, 1    ;; this is what we return

 _std_cont_pat_done
  call __builtin_quick_restore
  pop Ma
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
  push -1
  call __builtin_quick_save

  cmp Ma, _MSTD_NULL_
  je _std_cont_size_done

  add Ma, _MSTD_GET_COUNT_
  loadq Ma, Ma
  sss Ma, 1 ;; we return this

 _std_cont_size_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 1 for false, NULL for error else 0
;; NOTE: Not thread-safe and hence resizes may affect
__builtin_std_cont_is_empty
  push _MSTD_NULL_
  call __builtin_quick_save

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
  pop Ma
  ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 1 for false, NULL for error else 0
;; NOTE: Not thread-safe and hence resizes may affect
__builtin_std_cont_is_full
  push _MSTD_NULL_
  call __builtin_quick_save

  cmp Ma, _MSTD_NULL_
  je _std_cont_is_full_done

  add Ma, _MSTD_GET_COUNT_
  loadq M1, Ma
  add Ma, 8
  loadq M2, Ma
  cmp M1, M2
  je _std_cont_is_full_full

  movl Ma, 1
  sss Ma, 1
  jmp _std_cont_is_full_done

 _std_cont_is_full_full
  movl Ma, 0
  sss Ma, 1

 _std_cont_is_full_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 0 for error else the capacity
;; NOTE: Not thread-safe and hence resizes may affect
__builtin_std_cont_capacity
  push 0
  call __builtin_quick_save

  cmp Ma, _MSTD_NULL_
  je _std_cont_cap_done

  add Ma, _MSTD_GET_CAPACITY_
  loadq Ma, Ma
  sss Ma, 1

 _std_cont_cap_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 0 for error else the elen value
;; NOTE: Not thread-safe and hence resizes may affect
__builtin_std_cont_elen
  push 0
  call __builtin_quick_save

  cmp Ma, _MSTD_NULL_
  je _std_cont_elen_done

  add Ma, _MSTD_GET_ELEN_
  loadq Ma, Ma
  sss Ma, 1

 _std_cont_elen_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 1 for error else 0
;; THREAD-SAFE
__builtin_std_cont_erase
  push 1
  call __builtin_quick_save
   
  cmp Ma, _MSTD_NULL_
  je _std_cont_erase_done

  call __builtin_std_raw_acquire
  push Ma
  add Ma, _MSTD_GET_COUNT_
  xor Mb, Mb
  storeq Mb, Ma
  sss Mb, 1
  pop Ma
  call __builtin_std_raw_release
 _std_cont_erase_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to container
;; RETURNS: 1 for false, NULL for error else 0
__builtin_std_cont_is_dynamic
  push _MSTD_NULL_
  call __builtin_quick_save
   
  cmp Ma, _MSTD_NULL_
  je _std_cont_is_dyn_done

  add Ma, 1
  loadb Mb, Ma
  sss Mb, 1

 _std_cont_is_dyn_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to container
;; RETURNS: Nothing
;; NEEDS TO BE THREAD-SAFE
__builtin_std_cont_set_dynamic
  call __builtin_quick_save
   
  cmp Ma, _MSTD_NULL_
  je _std_cont_set_dyn_done

  call __builtin_std_raw_acquire
  add Ma, 1
  movl Mb, 1
  storeb Mb, Ma
  sub Ma, 1
  call __builtin_std_raw_release
 _std_cont_set_dyn_done
  call __builtin_quick_restore
  ret

;; ARGS: Ma = PTR to container
;; RETUNRS: Ma = realloc proc, Mb = free proc else Ma = NULL
__builtin_std_cont_get_allocator
   call __builtin_quick_save
   cmp Ma, _MSTD_NULL_
   movl Mb, _MSTD_NULL_
   sss Mb, 1
   je _std_cont_get_alloc_done

   add Ma, _MSTD_GET_FREE_PROC_
   loadq Mb, Ma
   sub Ma, 8
   loadq Ma, Ma
   sss Ma, 1
   sss Mb, 2
 _std_cont_get_alloc_done
   call __builtin_quick_restore
   ret

;; ARGS: Ma = PTR to a container, Mb = Find proc
;; RETURNS: Ma = 0 for success else 1
;; THREAD-SAFE
__builtin_std_cont_set_find
   push 1
   call __builtin_quick_save
   cmp Ma, _MSTD_NULL_
   je _std_cont_set_find_done

   call __builtin_std_raw_acquire
   push Ma
   add Ma, _MSTD_GET_FIND_PROC_
   storeq Mb, Ma
   xor Mm1, Mm1
   sss Mm1, 1
   pop Ma
   call __builtin_std_raw_release
 _std_cont_set_find_done
   call __builtin_quick_restore
   pop Ma
   ret

;; ARGS: Ma = PTR to a container, Mb = group search proc
;; RETURNS: Ma = 0 for success else 1
;; THREAD-SAFE
__builtin_std_cont_set_group_search
   push 1
   call __builtin_quick_save
   cmp Ma, _MSTD_NULL_
   je _std_cont_set_grp_search_done

   call __builtin_std_raw_acquire
   push Ma
   add Ma, _MSTD_GET_GRP_SEARCH_PROC_
   storeq Mb, Ma ;; Mb could very well be NULL
   xor Mm1, Mm1
   sss Mm1, 1
   pop Ma
   call __builtin_std_raw_release
 _std_cont_set_grp_search_done
   call __builtin_quick_restore
   pop Ma
   ret

;; ARGS: Ma = PTR to container, Mb = ptr to the element to push
;; RETUNRS: Ma = PTR to container else NULL
;; THREAD-SAFE
;; NOTE: With this procedure, the data to be pushed must have the correct endianness
__builtin_std_cont_push
  push _MSTD_NULL_
  call __builtin_quick_save
  cmp Ma, _MSTD_NULL_
  cmp Mb, _MSTD_NULL_
  je _std_cont_push_inval

  call __builtin_std_raw_acquire

  mov Mf, Ma ;; temporary storage
  add Ma, _MSTD_GET_ELEN_
  loadq M1, Ma ;; elen
  add Ma, 8
  loadq M2, Ma ;; count
  add Ma, 8 
  loadq M3, 8  ;; capacity

  cmp M2, M3
  js _std_cont_push_continue
  
  ;; we need to resize the container
  excgq Ma, Mf
  push Mb
  movl Mb, 2
  call __builtin_std_cont_resize
  pop Mb
  cmp Ma, _MSTD_NULL_
  je _std_cont_push_failed

  sss Ma, 1    ;; This can be safely done now(return)
  excgq Ma, Mf

 _std_cont_push_continue
  add Ma, 40
  loadq Ma, Ma ;; array

  ;; calculate offsets and write back
  mov Mc, M1
  add Ma, Mc 
  excgq Mb, Ma
  call __builtin_std_memcpy ;; This shouldn't fail at all

  ;; update the internals
  push Mf
  inc M2
  add Mf, _MSTD_GET_COUNT_
  storeq M2, Mf
  pop Mf

 _std_cont_push_failed
  mov Ma, Mf
  call __builtin_std_raw_release
  jmp _std_cont_push_done

 _std_cont_push_inval
  movl Ma, _M_EINVAL
  call __builtin_set_errno
  
 _std_cont_push_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to container, Mb = PTR to where the data needs to be stored
;; RETURNS: Ma = PTR to container else NULL
__builtin_std_cont_pop
  push _MSTD_NULL_
  call __builtin_quick_save
  cmp Ma, _MSTD_NULL_
  cmp Mb, _MSTD_NULL_
  je _std_cont_pop_inval

  call __builtin_std_raw_acquire
  push Ma
  call __builtin_std_cont_is_empty
  cmp Ma, 0
  pop Ma
  je _std_cont_pop_failed

  ;; It isn't empty and so we can just pop the value
  mov Mf, Ma  ;; temporary storage
  add Ma, _MSTD_GET_ELEN_
  loadq M1, Ma ;; elen
  add Ma, 8
  loadq M2, Ma ;; count
  add Ma, 48
  loadq M3, Ma ;; the array

  ;; calculate the offsets
  dec M2
  mov Mc, M2
  mul Mc, M1 ;; the offset
  add M3, Mc

  ;; finally pop
  mov Ma, M3 ;; source
  mov Mc, M1 ;; len
  call __builtin_std_memcpy ;; shouldn't fail

  ;; update the internals
  push Mf
  add Mf, _MSTD_GET_COUNT_
  storeq M2, Mf
  pop Ma
  sss Ma, 1
  jmp _std_cont_pop_failed ;; didn't fail though

 _std_cont_pop_inval
  movl Ma, _M_EINVAL
  call __builtin_set_errno
  jmp _std_cont_pop_done

 _std_cont_pop_failed
  call __builtin_std_raw_release
   
 _std_cont_pop_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to a container, Mb = index to erase
;; RETURNS: Ma = NULL for failure else container PTR
;; THREAD-SAFE
__builtin_std_cont_eraseat
  push _MSTD_NULL_
  call __builtin_quick_save
  cmp Ma, _MSTD_NULL_
  je _std_cont_eraseat_done

  call __builtin_std_raw_acquire
  mov Mf, Ma ;; temporary

  ;; get all needed internals
  add Ma, _MSTD_GET_ELEN_
  loadq M1, Ma ;; elen
  add Ma, 8
  loadq M2, Ma ;; count
  cmp M2, 0
  je _std_cont_eraseat_failed
  cmp M2, Mb
  jse _std_cont_eraseat_failed
  add Ma, 48
  loadq M3, Ma ;; array

  ;; make calculations
  mov Mm1, Mb
  mul Mm1, M1 ;; the offset of the element to be erased
  mov Mm2, Mm1
  add Mm2, M1 ;; the offset of the next element
  mov Mc, M2 
  dec Mc
  sub Mc, Mb
  mul Mc, M1 ;; The number of bytes to shift
  
  ;; perform the shift
  mov Ma, Mm2
  mov Mb, Mm1
  call __builtin_std_memcpy ;; won't fail
  
  ;; update internals
  push Mf
  add Mf, _MSTD_GET_COUNT_
  storeq Mc, Mf
  pop Mf
  sss Mf, 1
  jmp _std_cont_eraseat_failed ;; didn't fail though

 _std_cont_eraseat_inval
  movl Ma, _M_EINVAL
  call __builtin_set_errno
  jmp _std_cont_erase_done

 _std_cont_eraseat_failed
  mov Ma, Mf
  call __builtin_std_raw_release

 _std_cont_eraseat_done
  call __builtin_quick_restore
  pop Ma
  ret

;; ARGS: Ma = PTR to container(Lock of this is used), Mb = PTR to the next container(This isn't freed)
;; RETURNS: Ma = PTR to the first container else NULL
;; NOTE: The lock for the second container isn't used and it isn't freed afterwards. 
;;       The ELEN for the first container is used and for the second container isnt' even used.
;; THREAD-SAFE
__builtin_std_cont_append
  push _MSTD_NULL_
  call __builtin_quick_save
  cmp Ma, _MSTD_NULL_
  cmp Mb, _MSTD_NULL_
  je _std_cont_append_inval
  je _std_cont_append_inval

  call __builtin_std_raw_acquire
  mov Mf, Ma ;; temporary
  ;;mov Mf, Mb
  
  ;; gather all of the necessary values
  add Mb, _MSTD_GET_COUNT_
  loadq M2, Mb ;; count
  add Mb, 48
  loadq M3, Mb ;; array
  
  add Ma, _MSTD_GET_ELEN_
  loadq Mm1, Ma ;; elen
  add Ma, 8
  loadq Mm2, Ma ;; count
  add Ma, 8
  loadq Mm3, Ma ;; capacity

  ;; perform calculations
  ;; check if there is already enough space
  mov Mc, Mm3
  sub Mc, Mm2
  cmp Mc, M2
  jge _std_cont_append_enough
  
  ;; we don't have enough storage
  mov Ma, Mf
  ;;push Mb
  mov Mb, M2
  call __builtin_std_cont_capinc
  cmp Ma, _MSTD_NULL_ 
  je _std_cont_append_failed

 _std_cont_append_enough
  ;; we have enough storage
  add Ma, _MSTD_GET_ARRAY_
  loadq Mb, Ma ;; the array

  mov Mc, Mm2
  mul Mc, Mm1
  add Mb, Mc ;; to get to the correct offset
  mov Mc, M2
  mul Mc, Mm1 ;; total number of bytes
  mov Ma, M3
  call __builtin_std_memcpy ;; should not fail

  ;; update the internals
  push Mf
  add Mf, _MSTD_GET_COUNT_
  add Mm2, M2
  storeq Mm2, Mf
  pop Mf
  sss Mf, 1
 _std_cont_append_failed
  mov Ma, Mf
  call __builtin_std_raw_release
  jmp _std_cont_append_done

 _std_cont_append_inval
  movl Ma, _M_EINVAL
  call __builtin_set_errno

 _std_cont_append_done
  call __builtin_quick_restore
  pop Ma
  ret

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

;; ARGS: Ma = PTR to container, Mb = index, Mc = PTR to the element
;; RETURNS: Ma = PTR to container else NULL
;; THREAD-SAFE 
__builtin_std_cont_insert
  call __builtin_quick_save
  push Ma


 _std_cont_insert_failed
  pop Ma
  call __builtin_std_raw_release
 _std_cont_insert_done
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


;; Given how the internal allocator requires that the memory isn't meddled with, if you don't want your allocator's data to be overwritten, do not use it.
;; Any stdlib construct will allow you to specify a custom allocator which can do whatever it wants with the entire memory.
;; The stdlib allocator and a custom allocator may work together by letting the custom allocator suppress the stdlib allocator's influence to only the first 
;; pages we get. This, however, is useless since the things depending on the stdlib allocator won't get their requests fulfilled later on during program execution.
;; This means- it's either the stdlib allocator or the custom allocator.
;; This module requires the allocators to be initialized.

