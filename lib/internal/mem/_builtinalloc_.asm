depends _builtininit_.asm
depends _builtinintr_.asm
depends _builtinutils_.asm
depends _builtindefs_.asm

proc __builtin_std_mem_init
proc __builtin_std_alloc
proc __builtin_std_mem_check_free_blocks
proc __builtin_std_mem_request_more_mem
proc __builtin_std_free
proc __builtin_std_realloc
proc __builtin_std_memcpy
proc __builtin_std_memscpy   ;; Memory safe copy
proc __builtin_std_memset
proc __builtin_std_salloc    ;; Set alloc

proc __builtin_std_create_new_block

;; This is a quick error logging procedure with nothing fancy
proc __builtin_std_mem_log_err

;; Utility procedures
proc __builtin_std_mem_append_to_free      ;; used by __builtin_std_free
proc __builtin_std_mem_append_to_allocated   ;; used by __builtin_std_alloc
proc __builtin_std_mem_remove_from_free     ;; used by __builtin_std_alloc
proc __builtin_std_mem_remove_from_allocated ;; used by __builtin_std_free

proc __builtin_std_mem_populate_free_head      ;; used to fill the free head
proc __builtin_std_mem_populate_allocated_head  ;; used to fill allocated head

proc __builtin_std_mem_is_init       ;; has the allocator been initialized?
proc __builtin_std_mem_has_enough_mem

;; ARGS: None
;; RETURNS: Ma = 1 for TRUE else 0
__builtin_std_mem_is_init
   movl Ma, 0
   cmp Ma, _Mstd_num_of_pages
   je _std_mem_is_init_done

   movl Ma, 1

 _std_mem_is_init_done
   ret

;; ARGS: Ma = Size
;; RETURNS: Ma = 1 for TRUE else 0
__builtin_std_mem_has_enough_mem
   mov Mb, Ma
   movl Ma, 1
   cmp Mb, _Mstd_allocable_mem_len
   jse _std_mem_has_enough_mem_done

   movl Ma, 0

 _std_mem_has_enough_mem_done
   ret

;; ARGS: Ma = address to make the head
;; RETURNS: Nothing
__builtin_std_mem_populate_free_head
   
   ;; call to this procedure means that the free head is empty
   ;; and so is the tail
   ;; NXT must be NULL
   storeq Ma, _Mstd_free_mem_head
   movl Mb, Ma

   sub Ma, _MSTD_MEM_METADATA_NXT_
   storeq Mb, Ma
   sub Ma, _MSTD_MEM_METADATA_NXT_
   storeq Mb, Ma
   
   ret

;; ARGS: Ma = address to make the head
;; RETURNS: Nothing
__builtin_std_mem_populate_allocated_head
   
   ;; call to this procedure means that the allocated head is empty
   ;; and so is the tail
   ;; PREV and NXT must be NULL
   storeq Ma, _Mstd_allocated_mem_head
   movl Mb, Ma

   sub Ma, _MSTD_MEM_METADATA_NXT_
   storeq Mb, Ma
   sub Ma, _MSTD_MEM_METADATA_NXT_
   storeq Mb, Ma
   
   ret

;; ARGS: Ma = address for the block
;; RETURNS: Nothing
__builtin_std_mem_append_to_free

   ;; The block that was provided must have NULL for NXT and PREV
   call __builtin_quick_save
   
   loadq M1, _Mstd_free_mem_head

   cmp M1, _MSTD_NULL_
   je _std_mem_append_to_free_populate_head

   push M1
   sub M1, _MSTD_MEM_METADATA_PREV_
   loadq M2, M1
   pop M1

   push M2
   sub M2, _MSTD_MEM_METADATA_NXT_
   storeq Ma, M2
   pop M2

   push Ma
   sub Ma, _MSTD_MEM_METADATA_NXT_
   storeq M1, Ma
   sub Ma, _MSTD_MEM_METADATA_NXT_
   storeq M2, Ma
   pop Ma

   sub M1, _MSTD_MEM_METADATA_PREV_
   storeq Ma, M1

   jmp _std_mem_append_to_free_done 

 _std_mem_append_to_free_populate_head
   call __builtin_std_mem_populate_free_head

 _std_mem_append_to_free_done
   call __builtin_quick_restore
   ret

;; ARGS: Ma = address for the block
;; RETURNS: Nothing
__builtin_std_mem_append_to_allocated

   ;; The block that was provided must have NULL for NXT and PREV
   call __builtin_quick_save
   
   loadq M1, _Mstd_allocated_mem_head
   
   cmp M1, _MSTD_NULL_
   je _std_mem_append_to_allocated_populate_head

   push M1
   sub M1, _MSTD_MEM_METADATA_PREV_
   loadq M2, M1
   pop M1

   push M2
   sub M2, _MSTD_MEM_METADATA_NXT_
   storeq Ma, M2
   pop M2

   push Ma
   sub Ma, _MSTD_MEM_METADATA_NXT_
   storeq M1, Ma
   sub Ma, _MSTD_MEM_METADATA_NXT_
   storeq M2, Ma
   pop Ma

   sub M1, _MSTD_MEM_METADATA_PREV_
   storeq Ma, M1

   jmp _std_mem_append_to_free_done 

 _std_mem_append_to_allocated_populate_head
   call __builtin_std_mem_populate_allocated_head

 _std_mem_append_to_allocated_done
   call __builtin_quick_restore
   ret

;; ARGS: Ma = address for the block to remove
;; RETURNS: Nothing
__builtin_std_mem_remove_from_free
   call __builtin_quick_save

   ;; Updates the Free List and makes NXT and PREV NULL
   movl Mb, Ma          ;; temporary storage
   
   movl Mm1, _MSTD_NULL_
   
   sub Mb, _MSTD_MEM_METADATA_NXT_
   loadq M1, Mb
   storeq Mm1, Mb
   sub Mb, _MSTD_MEM_METADATA_NXT_
   loadq M2, Mb
   storeq Mm1, Mb

   ;; M1 = NEXT block and M2 = PREVIOUS block

   ;; If Ma is the head block and also if that is the only block
   ;; This condition is fulfilled only if Ma is the head 
   cmp M1, M2
   je _std_mem_remove_from_free_is_only_head

   jmp _std_mem_remove_from_free_continue 

 _std_mem_remove_from_free_is_only_head
   push _MSTD_NULL_
   pop _Mstd_free_mem_head

   jmp _std_mem_remove_from_free_done
   
 _std_mem_remove_from_free_continue

   ;; The block is not the head or we have more blocks
   push M1
   sub M1, _MSTD_MEM_METADATA_PREV_
   storeq M2, M1
   pop M1

   sub M2, _MSTD_MEM_METADATA_NXT_
   storeq M1, M2
   
 _std_mem_remove_from_free_done
   call __builtin_quick_restore
   ret

;; ARGS: Ma = address for the block to remove
;; RETURNS: Nothing
__builtin_std_mem_remove_from_allocated
   call __builtin_quick_save

   ;; Updates the Allocated List and makes NXT and PREV NULL
   movl Mb, Ma          ;; temporary storage
   
   movl Mm1, _MSTD_NULL_
   storeq M1, Mm1
   sub Mb, _MSTD_MEM_METADATA_NXT_
   loadq M1, Mb
   storeq Mm1, Mb
   sub Mb, _MSTD_MEM_METADATA_NXT_
   loadq M2, Mb
   storeq Mm1, Mb

   ;; M1 = NEXT block and M2 = PREVIOUS block

   ;; If Ma is the head block and also if that is the only block
   ;; This condition is fulfilled only if Ma is the head 
   cmp M1, M2
   je _std_mem_remove_from_allocated_is_only_head

   jmp _std_mem_remove_from_allocated_continue 

 _std_mem_remove_from_allocated_is_only_head
   push _MSTD_NULL_
   pop _Mstd_allocated_mem_head

   jmp _std_mem_remove_from_allocated_done
   
 _std_mem_remove_from_allocated_continue

   ;; The block is not the head or we have more blocks
   push M1
   sub M1, _MSTD_MEM_METADATA_PREV_
   storeq M2, M1
   pop M1

   sub M2, _MSTD_MEM_METADATA_NXT_
   storeq M1, M2
   
 _std_mem_remove_from_allocated_done
   call __builtin_quick_restore
   ret

;; Initialize memory system.
;; ARGS: None, RETURN: Nothing
__builtin_std_mem_init
   call __builtin_quick_save
   
   ;; Initialize internal memory management values
   loadq Ma, _Mstd_free_mem_start_addr
   storeq Ma, _Mstd_allocable_mem_start
   storeq Ma, _Mstd_allocable_mem_end

   ;; Set initial pointers to NULL
   movl Ma, _MSTD_NULL_
   storeq Ma, _Mstd_allocated_mem_head
   storeq Ma, _Mstd_free_mem_head

   ;; Initialize allocated memory count
   movl Ma, 0
   storeq Ma, _Mstd_allocated_mem

   ;; Calculate total memory size
   loadq Ma, _Mstd_num_of_pages
   mul Ma, _MSTD_MEM_PAGE_LEN
   sub Ma, _Mstd_allocable_mem_start
   storeq Ma, _Mstd_allocable_mem_len     ;; Store available memory size

   call __builtin_quick_restore
   ret

;; ARGS: Ma = PTR to message, Mb= length of msg
__builtin_std_mem_log_err
   mov Mc, Mb
   sout Ma
   mov Ma, 10
   cout Ma
   mov Ma, 1          ;; Unsuccessful return
   intr _M_EXIT_       ;; The entire VM stops(unless child processes are running)

;; ARGS: Nothing
;; RETURNS: 1 for success and 0 for failure
__builtin_std_mem_request_more_mem
   call __builtin_quick_save
   push 0
   pop _Mstd_mem_intermediate
   intr _M_MEM_                    ;; ask for more memory
   cmp Ma, 1
   je _std_mem_request_more_mem_done

   inc Ma
   mul Ma, _MSTD_MEM_PAGE_LEN

   loadq M1, _Mstd_allocable_mem_end
   loadq M2, _Mstd_allocable_mem_len
   loadq M3, _Mstd_num_of_pages

   inc M3
   storeq M3, _Mstd_num_of_pages
   add M2, Ma
   storeq M2, _Mstd_allocable_mem_len
   add M1, Ma
   storeq M1, _Mstd_allocable_mem_end
   
   push 1
   pop _Mstd_mem_intermediate

 _std_mem_request_more_mem_done
   call __builtin_quick_restore
   loadq Ma, _Mstd_mem_intermediate
   ret

;; ARGS: Ma = Size of block to check
;; RETURNS: Ma = NULL for no match else a valid address
__builtin_std_mem_check_free_blocks

   call __builtin_quick_save

   push _MSTD_NULL_
   pop _Mstd_mem_intermediate
   
   loadq M1, _Mstd_free_mem_head

   cmp M1, _MSTD_NULL_
   je _std_mem_check_free_blocks_done
   
   mov M2, M1
 _std_mem_check_free_blocks_loop
   push M1
   sub M1, _MSTD_MEM_METADATA_BLOCK_LEN
   loadq Mm1, M1
   cmp Mm1, Ma
   jge _std_mem_check_free_blocks_found

   pop M1
   sub M1, _MSTD_MEM_METADATA_NXT_
   loadq Mm1, M1
   cmp Mm1, _MSTD_NULL_
   je _std_mem_check_free_blocks_done ;; NULL- found None

   jmp _std_mem_check_free_blocks_loop ;; REDO- again and again until the end

 _std_mem_check_free_blocks_found
   pop _Mstd_mem_intermediate

 _std_mem_check_free_blocks_done
   call __builtin_quick_restore
   loadq Ma, _Mstd_mem_intermediate
   ret

;; ARGS: Ma = Size of block to create
;; RETURNS: Ma = newly allocated block or NULL
__builtin_std_create_new_block
   call __builtin_quick_save
   
   push _MSTD_NULL_
   pop _Mstd_mem_intermediate

   mov Mm5, Ma

   ;; check if we have enough memory first
   call __builtin_std_mem_has_enough_mem
   cmp Ma, 0
   jne _std_create_new_block_allocate

   call __builtin_std_mem_request_more_mem
   cmp Ma, 0
   je _std_create_new_block_done         ;; we failed

 _std_create_new_block_allocate
 
   loadq M1, _Mstd_allocable_mem_end
   push M1
   movl Mm1, _MSTD_MEM_METADATA_LEN_
   add Mm1, Mm5
   add M1, Mm1
   storeq M1, _Mstd_allocable_mem_end ;; update internals

   loadq M1, _Mstd_allocable_mem_len
   sub M1, Mm1
   storeq M1, _Mstd_allocable_mem_len
   loadq M1, _Mstd_allocated_mem
   add M1, Mm1
   storeq M1, _Mstd_allocated_mem

   pop M1
   add M1, _MSTD_MEM_METADATA_LEN_ ;; M1 now has the address we have to return
   push M1
   pop _Mstd_mem_intermediate

   movl Mf, _MSTD_NULL_
   sub M1, _MSTD_MEM_METADATA_NXT_
   storeq Mf, M1
   sub M1, _MSTD_MEM_METADATA_NXT_
   storeq Mf, M1
   sub M1, _MSTD_MEM_METADATA_NXT_
   storeq Mm5, M1

   sub M1, _MSTD_MEM_METADATA_NXT_
   movl Mf, _MSTD_MEM_MAGIC_NUM_
   storeq Mf, M1

 _std_create_new_block_done
   call __builtin_quick_restore
   loadq Ma, _Mstd_mem_intermediate
   
   ret

;; ARGS: Ma = Size of block to allocate
;; RETURNS: Ma = NULL for failure else a valid pointer
__builtin_std_alloc
   call __builtin_quick_save
   
   push _MSTD_NULL_
   pop _Mstd_mem_intermediate

   ;; Align the size first
   call __builtin_align_value

   cmp Ma, 0
   je _std_alloc_done

   ;; Check if the allocator has been initialized
   movl Mf, Ma
   call __builtin_std_mem_is_init
   cmp Ma, 0
   je _std_alloc_done         ;; Allocator not initialized
   
   movl Ma, Mf
   ;; Check if we have a free block that matches the requested size
   movl Mb, Ma
   call __builtin_std_mem_check_free_blocks
   cmp Ma, _MSTD_NULL_
   jne _std_alloc_found_free_block

   ;; No free block found, create a new block
   movl Ma, Mb
   call __builtin_std_create_new_block
   cmp Ma, _MSTD_NULL_
   je _std_alloc_done         ;; Failed to create new block
   jmp _std_alloc_add_to_allocated_list

 _std_alloc_found_free_block
   ;; Remove the free block from the free list
   call __builtin_std_mem_remove_from_free

 _std_alloc_add_to_allocated_list
   ;; Add the allocated block to the allocated list
   call __builtin_std_mem_append_to_allocated

 _std_alloc_done
   call __builtin_quick_restore
   loadq Ma, _Mstd_mem_intermediate
   ret
   
;; ARGS: Ma = Address of the block to remove
;; RETURNS: Nothing but will throw an error and exit for invalid pointer
__builtin_std_free
   call __builtin_quick_save

   movl Mf, Ma

   ;; Check if the allocator has been initialized
   call __builtin_std_mem_is_init
   cmp Ma, 0
   je _std_free_error         ;; Allocator not initialized

   movl Ma, Mf
   ;; Check if the block is valid
   sub Ma, _MSTD_MEM_METADATA_MAGIC_NUM_
   loadq M1, Ma
   cmp M1, _MSTD_MEM_MAGIC_NUM_
   jne _std_free_error         ;; Invalid block

   ;; Remove the block from the allocated list
   mov Ma, Mf
   call __builtin_std_mem_remove_from_allocated

   ;; Add the block to the free list
   call __builtin_std_mem_append_to_free
   call __builtin_quick_restore
   ret

 _std_free_error
   mov Ma, [PTR _MSTD_INVALID_FREE]
   movl Mc, 13
   call __builtin_std_mem_log_err


;; ARGS: Ma = Address to the memory, Mb = Size of each member, Mc = Number of members, Md = The value to set
;; RETURNS: Ma = Address on success else NULL
;; NOTE: Mb can only be as large as 8 and cannot be 0
__builtin_std_memset
   call __builtin_quick_save
   
   push _MSTD_NULL_
   pop _Mstd_mem_intermediate

   cmp Ma, _MSTD_NULL_
   je _std_memset_done

   cmp Mb, 0
   je _std_memset_done

   cmp Mb, 9
   jge _std_memset_done

   cmp Mc, 0
   push Ma
   pop _Mstd_mem_intermediate

   je _std_memset_done
   movl Mf, Md

   ;; Mc already has the number of members set 

 _std_memset_loop
   push Mc
   mov Mf, 0
 _std_memset_innerloop
   mov Mc, Mb
 defined _M_AMD_
   ;; For Little endian machines
   ;; Put in little endian format as well
   mov Mm1, Md
   mov Mm2, Mf
   mul Mm2, 8
   rshift Mm1, Mm2
   and Mm1, 0xFF
 end
   storeb Mm1, Ma
   inc Ma
   inc Mf
   loop _std_memset_innerloop
   pop Mc
   loop _std_memset_loop

 _std_memset_done
   call __builtin_quick_restore
   loadq Ma, _Mstd_mem_intermediate
   ret
  
;; ARGS: Ma = Source address, Mb = Destination address, Mc = Number of bytes
;; RETURN: Ma = Mb for Success else NULL
__builtin_std_memcpy
  call __builtin_quick_save

  push _MSTD_NULL_
  pop _Mstd_mem_intermediate

  cmp Ma, _MSTD_NULL_
  je _std_memcpy_done

  cmp Mb, _MSTD_NULL_
  je _std_memcpy_done

  cmp Mc, 0
  push Mb
  pop _Mstd_mem_intermediate
  je _std_memcpy_done

 _std_memcpy_loop
  loadb M1, Ma
  storeb M1, Mb
  inc Ma
  inc Mb
  loop _std_memcpy_loop

 _std_memcpy_done
  call __builtin_quick_restore
  loadq Ma, _Mstd_mem_intermediate
  ret

;; ARGS: Ma = Old address, Mb = New size
;; RETURN: Ma = A new pointer on success else NULL
__builtin_std_realloc
  call __builtin_quick_save

  ;; We will first allocate new memory with the requested number of bytes and then
  ;; We will memcpy the old data into the new one and free the old one.
  push _MSTD_NULL_
  pop _Mstd_mem_intermediate

  cmp Mb, 0
  je _std_realloc_continue

  mov Mm1, Ma
  mov Ma, Mb
  call __builtin_std_alloc

  cmp Ma, _MSTD_NULL_
  je _std_realloc_done     ;; we failed
  mov Mb, Ma ;; The new block
  mov Ma, Mm1  ;; The old block
  mov Mm2, Mm1 ;; The old block
  sub Mm1, _MSTD_MEM_METADATA_BLOCK_LEN ;; get the old block's length
  loadq Mc, Mm1
  call __builtin_std_memcpy
  cmp Ma, _MSTD_NULL_
  push Ma
  pop _Mstd_mem_intermediate
  je _std_realloc_done

 _std_realloc_continue
  mov Ma, Mm2
  call __builtin_std_free

 _std_realloc_done
  call __builtin_quick_restore
  loadq Ma, _Mstd_mem_intermediate
  ret
;; ARGS: Ma = Size of members, Mb = Number of members
;; RETURN: Ma = A newly allocated block else NULL for error
;; NOTE: salloc is the same as 'calloc' from C stdlib. All the bytes are initialized to 0
__builtin_std_salloc
  call __builtin_quick_save

  push _MSTD_NULL_
  pop _Mstd_mem_intermediate

  mul Ma, Mb
  jo _std_salloc_done

  mov Mm1, Ma
  call __builtin_std_alloc
  cmp Ma, _MSTD_NULL_
  je _std_salloc_done

  mov Mb, 1
  mov Mc, Mm1
  xor Md, Md
  call __builtin_std_memset

  push Ma
  pop _Mstd_mem_intermediate

 _std_salloc_done
  call __builtin_quick_restore
  loadq Ma, _Mstd_mem_intermediate
  ret
;; ARGS: Ma = Source address, Mb = Destination address, Mc = Number of bytes
;; RETURN: Ma = Mb else NULL for error
;; NOTE: memscpy is the same as 'memmove' from C-stdlib
__builtin_std_memscpy
  call __builtin_quick_save

  push _MSTD_NULL_
  pop _Mstd_mem_intermediate

  cmp Ma, _MSTD_NULL_
  je _std_memscpy_done
   
  cmp Mb, _MSTD_NULL_
  je _std_memscpy_done

  push Mb
  pop _Mstd_mem_intermediate

  cmp Mc, 0
  je _std_memscpy_done

  cmp Ma, Mb
  jge _std_memscpy_dest_first

  ;; The destination comes later than the Source
  ;; we just copy from the last byte

  add Ma, Mc
  add Mb, Mc

 _std_memscpy_loop
  loadb M1, Ma
  storeb M1, Mb
  dec Ma
  dec Mb
  loop _std_memscpy_loop
  
  jmp _std_memscpy_done

 _std_memscpy_dest_first
  ;; in case the destination address is first, we will just copy normally
  call __builtin_std_memcpy

 _std_memscpy_done
  call __builtin_quick_restore
  loadq Ma, _Mstd_mem_intermediate
  ret

;; The Data
rq _Mstd_allocable_mem_len 1   ;; The memory that we can allocate with
rq _Mstd_allocated_mem 1       ;; The memory that we have already allocated  

rq _Mstd_allocable_mem_start 1 ;; The start of the free memory
rq _Mstd_allocable_mem_end 1   ;; The end of the total occupied memory(Doesn't indicate the end of free memory)
rq _Mstd_allocated_mem_head 1  ;; Pointer to the first allocated byte
rq _Mstd_free_mem_head 1       ;; Pointer to the free head

rq _Mstd_mem_intermediate 1    ;; This is something that shouldn't be used by other procedures

;; Each allocated block is gonna have the following metadata 
;; QWORD magic_num  ;; An identifying value
;; QWORD _block_len ;; The number of allocated bytes
;; QWORD* nxt       ;; Pointer to the next block

dc _MSTD_MEM_MAGIC_NUM_ 0xAABBCCDDEEFF9977

dc _MSTD_MEM_METADATA_LEN_ 32
dc _MSTD_MEM_METADATA_MAGIC_NUM_ 32
dc _MSTD_MEM_METADATA_BLOCK_LEN 24
dc _MSTD_MEM_METADATA_PREV_ 16
dc _MSTD_MEM_METADATA_NXT_ 8

dc _MSTD_MEM_PAGE_LEN 0x100000 ;; The length of one page

ds _MSTD_INVALID_FREE "Invalid free".0 ;; This is the only error message we have

;; We cannot merge two blocks together to get a bigger block because in the doubly linked list,
;; we cannot be sure that any two blocks are right next to one another in memory. 
;; It will not be smart to attempt to check for any two blocks and hence we will avoid that
;; entirely.