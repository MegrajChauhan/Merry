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

;; Procedures marked with ERRNO set errno on some errors

depends _builtincont_.asm

proc std::cont::create
proc std::cont::destroy
proc std::cont::resize
proc std::cont::cap_increase
proc std::cont::ptr_first
proc std::cont::ptr_last
proc std::cont::ptr_at
proc std::cont::ptr_of ;; EXACTLY the same as ptr_at, why? I don't know.
proc std::cont::csize
proc std::cont::is_empty
proc std::cont::is_full
proc std::cont::ccapacity
proc std::cont::celen
proc std::cont::erase
proc std::cont::is_dynamic
proc std::cont::make_dynamic
proc std::cont::get_alloc
proc std::cont::set_find_proc
proc std::cont::set_grp_search_proc
proc std::cont::push
proc std::cont::pop
proc std::cont::erase_at
proc std::cont::append
proc std::cont::insert
proc std::cont::find
proc std::cont::grp_search

;; In case there are no provided procedures, the builtin ones will be used.

;; ARGS: Ma = ELEN, Mb = FLAG for dynamic(1 for true else 0), Mc = 0 for custom allocator else anything
;; If Mc == 0 then:
;; Md = alloc_proc, M1 = find_proc, M2 = group_search_proc, M3 = realloc_proc, M4 = free_proc   
;; RETURNS: Ma = NULL for failure else a pointer to the new container
;; ERRNO
std::cont::create
    call __builtin_std_cont_create
    ret

;; ARGS: Ma = PTR to the container
;; RETURNS: Nothing
;; NOTE: Make sure that every single thread has done what it needs before doing this
std::cont::destroy
    call __builtin_std_cont_destroy
    ret

;; ARGS: Ma = PTR to the container, Mb = Factor
;; RETURNS: Ma = PTR to the container else NULL
;; NOTE: The container is resized by a factor of FACTOR
;; After each resize, the pointers that you may have for the elements will
;; be invalidated and hence you should not access them
;; THREAD-SAFE, ERRNO
std::cont::resize
    call __builtin_std_cont_resize
    ret

;; ARGS: Ma = PTR to the container, Mb = Capacity to add
;; RETURNS: Ma = PTR to the container else NULL
;; NOTE: The container's capacity is increased by Mb elements only
;; After this, the pointers that you may have for the elements will
;; be invalidated and hence you should not access them
;; THREAD-SAFE, ERRNO
std::cont::cap_increase
    call __builtin_std_cont_capinc
    ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = PTR to the first element else NULL for no elements pushed or invalid argument
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
std::cont::ptr_first
    call __builtin_std_cont_pfirst
    ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = PTR to the last element else NULL for no elements pushed or invalid argument
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
std::cont::ptr_last
    call __builtin_std_cont_plast
    ret

;; ARGS: Ma = PTR to a container, Mb = index
;; RETURNS: Ma = PTR to the element at index else NULL for error or invalid argument
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
std::cont::ptr_at
    call __builtin_std_cont_pat
    ret

;; ARGS: Ma = PTR to a container, Mb = index
;; RETURNS: Ma = PTR to the element at index else NULL for error or invalid argument
;; NOTE: This isn't thread safe and hence if a resize is done then the
;; pointer will become invalid.
std::cont::ptr_of
    call __builtin_std_cont_at
    ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = The number of elements else -1 for invalid arguments
;; NOTE: Not thread-safe and hence resizes may affect
std::cont::csize
    call __builtin_std_cont_size
    ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 1 for false, NULL for error else 0
;; NOTE: Not thread-safe and hence resizes may affect
std::cont::is_empty
    call __builtin_std_cont_is_empty
    ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 1 for false, NULL for error else 0
;; NOTE: Not thread-safe and hence resizes may affect
std::cont::is_full
    call __builtin_std_cont_is_full
    ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 0 for error else the capacity
;; NOTE: Not thread-safe and hence resizes may affect
std::cont::ccapacity
    call __builtin_std_cont_capacity
    ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 0 for error else the elen value
;; NOTE: Not thread-safe and hence resizes may affect
std::cont::celen
    call __builtin_std_cont_elen
    ret

;; ARGS: Ma = PTR to a container
;; RETURNS: Ma = 1 for error else 0
;; THREAD-SAFE
std::cont::erase
    call __builtin_std_cont_erase
    ret

;; ARGS: Ma = PTR to container
;; RETURNS: 1 for false, NULL for error else 0
std::cont::is_dynamic
    call __builtin_std_cont_is_dynamic
    ret

;; ARGS: Ma = PTR to container
;; RETURNS: Nothing
;; THREAD-SAFE
std::cont::make_dynamic
    call __builtin_std_cont_set_dynamic
    ret

;; ARGS: Ma = PTR to container
;; RETUNRS: Ma = realloc proc, Mb = free proc else Ma = NULL
std::cont::get_alloc
    call __builtin_std_cont_get_allocator
    ret

;; ARGS: Ma = PTR to a container, Mb = Find proc
;; RETURNS: Ma = 0 for success else 1
;; THREAD-SAFE
std::cont::set_find_proc
    call __builtin_std_cont_set_find
    ret

;; ARGS: Ma = PTR to a container, Mb = group search proc
;; RETURNS: Ma = 0 for success else 1
;; THREAD-SAFE
std::cont::set_grp_search_proc
    call __builtin_std_cont_set_group_search
    ret

;; ARGS: Ma = PTR to container, Mb = PTR to the element to be pushed
;; RETURNS: Ma = PTR to container else NULL
;; THREAD-SAFE, ERRNO
std::cont::push
    call __builtin_std_cont_push
    ret

;; ARGS: Ma = PTR to container, Mb = PTR to where the data needs to be stored
;; RETURNS: Ma = PTR to container else NULL
;; THREAD-SAFE, ERRNO
std::cont::pop
    call __builtin_std_cont_pop
    ret

;; ARGS: Ma = PTR to a container, Mb = index to erase
;; RETURNS: Ma = NULL for failure else container PTR
;; THREAD-SAFE, ERRNO
std::cont::erase_at
    call __builtin_std_cont_eraseat
    ret

;; ARGS: Ma = PTR to container(Lock for this will be used)
;;       Mb = PTR to the next container(This isn't freed)
;; RETURNS: Ma = PTR to the first container else NULL
;; NOTE: The lock for the second container isn't used and it isn't freed afterwards. 
;;       The ELEN for the first container is used and for the second container isnt' even used.
;; THREAD-SAFE(for the first container), ERRNO
std::cont::append
    call __builtin_std_cont_append
    ret

;; ARGS: Ma = PTR to container, Mb = index, Mc = PTR to the element
;; RETURNS: Ma = PTR to container else NULL
;; THREAD-SAFE, ERRNO 
std::cont::insert
    call __builtin_std_cont_insert
    ret

;; ARGS: Ma = PTR to container, Mb = PTR to the element that needs to be found
;; RETURNS: Ma = PTR to the first byte of the element if found else NULL
;; THREAD-SAFE, ERRNO
std::cont::find
    call __builtin_std_cont_find
    ret

;; ARGS: Ma = PTR to container, Mb = PTR to the array that needs to be found, Mc = length of Mb
;; RETURNS: Ma = PTR to the first byte of the array if found else NULL
;; THREAD-SAFE, ERRNO
std::cont::grp_search
    call __builtin_std_cont_group_search
    ret