/*
 * Multithreading abstraction for the Merry VM
 * MIT License
 *
 * Copyright (c) 2023 MegrajChauhan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _MERRY_THREADSABS_
#define _MERRY_THREADSABS_

#include <stdlib.h> // this module is allowed to use malloc
#include "../../utils/merry_config.h"
#include "../../utils/merry_types.h"

typedef struct MerryMutex MerryMutex;
typedef struct MerryCond MerryCond;
typedef struct MerryThread MerryThread;

#if defined(_MERRY_HOST_OS_LINUX_)
#include "merry_thread_posix.h"
#define _MERRY_THREADS_POSIX_ 1
#endif

_MERRY_DEFINE_FUNC_PTR_(void *, ThreadExecFunc, void *)

// initialize mutex lock
_MERRY_NO_DISCARD_ MerryMutex *merry_mutex_init();
// initialize condition variable
_MERRY_NO_DISCARD_ MerryCond *merry_cond_init();
// initialize thread
_MERRY_NO_DISCARD_ MerryThread *merry_thread_init(); // this only initializes the thread but doesn't start the thread
// destroy mutex lock
void merry_mutex_destroy(MerryMutex *mutex);
// destroy condition variable
void merry_cond_destroy(MerryCond *cond);
// destory thread
void merry_thread_destroy(MerryThread *thread);

// lock mutex
void merry_mutex_lock(MerryMutex *mutex);
// unlock mutex
void merry_mutex_unlock(MerryMutex *mutex);
// condition wait
void merry_cond_wait(MerryCond *cond, MerryMutex *lock);
// condition signal
void merry_cond_signal(MerryCond *cond);
// create detached thread
mret_t merry_create_detached_thread(MerryThread *thread, ThreadExecFunc func, void *arg);
// create thread with no atrributes
mret_t merry_create_thread(MerryThread *thread, ThreadExecFunc func, void *arg);
// join thread with the calling thread
mret_t merry_thread_join(MerryThread *thread, void *return_val);

#endif