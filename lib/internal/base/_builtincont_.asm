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
proc __builtin_std_cont_set_allocator ;; set a custom allocator to be used(It must take the same arguments as the standard functions)
                                      ;; Once an element has been pushed, you may not set an allocator
                                      ;; In fact, once the internal allocator has been used, you may not use your own allocator
proc __builtin_std_cont_set_find ;; Since the container compares every byte when using __builtin_std_cont_find, some data structures might have a different requirement
                                 ;; Use this to provide the required comparison procedure
proc __builtin_std_cont_set_group_search

;; Given how the internal allocator requires that the memory isn't meddled with, if you don't want your allocator's data to be overwritten, do not use it.
;; Any stdlib construct will allow you to specify a custom allocator which can do whatever it wants with the entire memory.
;; The stdlib allocator and a custom allocator may work together by letting the custom allocator suppress the stdlib allocator's influence to only the first 
;; pages we get. This, however, is useless since the things depending on the stdlib allocator won't get their requests fulfilled later on during program execution.
;; This means- it's either the stdlib allocator or the custom allocator.
;; This module requires the allocators to be initialized.

