#ifndef _MERRY_THREADSABS_
#define _MERRY_THREADSABS_

#include <merry_utils.h>
#include <merry_types.h>
#include <pthread.h> // this is assuming that the host system is unix-based posix compliant system
#include <stdlib.h>
#include "merry_errno.h"
#include "merry_console.h"

typedef struct MerryMutex MerryMutex;
typedef struct MerryCond MerryCond;
typedef struct MerryThread MerryThread;

/*Mutex*/
struct MerryMutex
{
    pthread_mutex_t mutex;
};

/*Condition variables*/
struct MerryCond
{
    pthread_cond_t cond;
};

/*Semaphores can be implemented when needed*/
/*Threads*/
struct MerryThread
{
    pthread_t thread;
};

_MERRY_DEFINE_FUNC_PTR_(_THRET_T_, ThreadExecFunc, void *);

// initialize mutex lock
MerryMutex *merry_mutex_init();
// initialize condition variable
MerryCond *merry_cond_init();
// initialize thread
MerryThread *merry_thread_init(); // this only initializes the thread but doesn't start the thread
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
// broadcast signal
void merry_cond_broadcast(MerryCond *cond);
// create detached thread
mret_t merry_create_detached_thread(MerryThread *thread, ThreadExecFunc func, void *arg);
// create thread with no atrributes
mret_t merry_create_thread(MerryThread *thread, ThreadExecFunc func, void *arg);
// join thread with the calling thread
mret_t merry_thread_join(MerryThread *thread, void *return_val);

#endif