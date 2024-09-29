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

depends _builtinmp_.asm

proc std::mp::spawn

;; RETURNS: Ma = 1 for error else 0
std::mp::spawn
    call __builtin_std_spawn_process
    ret

;; NOTES:
;; The way Linux handles multi-processing via fork is extremely risky.
;; The preferred way, for me at least, is the window's multi-processing.
;; Taking that into account, Merry also does things exactly like Windows.
;; The calling thread is replicated and only that is replicated, however,
;; The new thread in the child process starts executing from the same entry address
;; as the calling thread.
;; For eg: Say three threads are running and one of the threads makes a request
;; for a new process. Assume that T1 started from 'main' procedure, T2 started from
;; 'proc1' procedure and T2 started from 'proc2' procedure. If T2 makes the request,
;; other two threads won't be spawned while T2 which is spawned will start executing from
;; the start of the 'proc2' procedure i.e all of the initializations will need to be
;; re-done. The memory is also copied, no file descriptors, locks or condition
;; variables are passed. The loaded library are also not passed.
;; The new process starts anew including the data memory as well.