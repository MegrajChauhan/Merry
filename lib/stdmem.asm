depends _builtinalloc_.asm

proc std::mem::init_allocator
proc std::mem::alloc
proc std::mem::free
proc std::mem::break
proc std::mem::os_alloc
proc std::mem::pg_count
proc std::mem::realloc
proc std::mem::memset
proc std::mem::memcpy
proc std::mem::salloc
proc std::mem::memscpy

std::mem::init_allocator
    call __builtin_std_mem_init
    ret

;; ARGS: Ma = Size of block to allocate
;; RETURNS: Ma = NULL for failure else a valid pointer
;; Thread-safe
std::mem::alloc
    call __builtin_std_alloc
    ret

;; ARGS: Ma = Address of the block to remove
;; RETURNS: Nothing but will throw an error and exit for invalid pointer
;; Thread-safe
std::mem::free
    call __builtin_std_free
    ret

;; The same as the sbrk system call in linux
;; Anyone wishing to build a custom memory allocator should use this to setup the internal metadata
;; Make sure that the builtin allocator is never invoked otherwise there will be problems
;; NOTE: Make sure to call std::init first
;; RETURNS: Ma = The start point, Mb = The end point 
std::mem::break
    loadq Ma, _Mstd_allocable_mem_start
    loadq Mb, _Mstd_allocable_mem_end
    ret    

;; Get the number of pages the program has that can be freely manipulated
;; RETURNS: Ma = number of pages
std::mem::pg_count
   loadq Ma, _Mstd_num_of_pages
   ret

;; This will ask the VM to allocate one more page
;; The creator of the custom allocator should be well aware that each page is 1024KB is size and each byte is addressable.
;; Calculate how many addresses that becomes
;; Make sure to call 'break' once again to receive the new end point and 'pg_count' to get new info
;; RETURNS: 1 for success and 0 for failure
std::mem::os_alloc
   call __builtin_std_mem_request_more_mem
   ret

;; ARGS: Ma = Old address, Mb = New size
;; RETURN: Ma = A new pointer on success else NULL
;; NOTE: Mb = 0 means the same as calling free
;; Thread-safe
std::mem::realloc
    call __builtin_std_realloc
    ret

;; ARGS: Ma = Address to the memory, Mb = Size of each member, Mc = Number of members, Md = The value to set
;; RETURNS: Ma = Address on success else NULL
;; NOTE: Mb can only be as large as 8 and cannot be 0
std::mem::memset
    call __builtin_std_memset
    ret

;; ARGS: Ma = Source address, Mb = Destination address, Mc = Number of bytes
;; RETURN: Ma = Mb for Success else NULL
std::mem::memcpy
    call __builtin_std_memcpy
    ret

;; ARGS: Ma = Size of members, Mb = Number of members
;; RETURN: Ma = A newly allocated block else NULL for error
;; NOTE: salloc is the same as 'calloc' from C stdlib. All the bytes are initialized to 0
;; Thread-safe
std::mem::salloc
    call __builtin_std_salloc
    ret

;; ARGS: Ma = Source address, Mb = Destination address, Mc = Number of bytes
;; RETURN: Ma = Mb else NULL for error
;; NOTE: memscpy is the same as 'memmove' from C-stdlib
std::mem::memscpy
    call __builtin_std_memscpy
    ret