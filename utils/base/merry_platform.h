#ifndef _MERRY_PLATFORM_
#define _MERRY_PLATFORM_

#include <merry_config.h>
#include <merry_types.h>
#include <merry_utils.h>

#ifdef _USE_LINUX_
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>

typedef pthread_t mthread_t;
typedef pthread_mutex_t mmutex_t;
typedef pthread_cond_t mcond_t;
typedef int mdataline_t;

#define merry_get_memory_page(size, prot, flags) mmap((void *)-1, size, PROT_READ | PROT_WRITE | prot, MAP_ANONYMOUS | MAP_PRIVATE | flags, -1, 0)
#define merry_return_memory_page(addr, size) munmap(addr, size)
#define _MERRY_FAILED_TO_GET_PAGE_ MAP_FAILED // failed to get memory
#define _MERRY_FAILED_TO_RETURN_PAGE_ -1      // the error sent by munmap on failure
#define _MERRY_PROT_DEFAULT_ 0x00             // default protection flag
#define _MERRY_FLAG_DEFAULT_ 0x00             // default flag

#elif defined(_USE_WIN_)
#include <windows.h>

typedef HANDLE mthread_t;
typedef CRITICAL_SECTION mmutex_t;
typedef CONDITION_VARIABLE mcond_t;
typedef HANDLE mdataline_t;

#define merry_get_memory_page(size, prot, flags) VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
#define merry_return_memory_page(addr, size) VirtualFree(addr, 0, MEM_RELEASE)
#define _MERRY_FAILED_TO_GET_PAGE_ NULL
#define _MERRY_FAILED_TO_RETURN_PAGE_ 0

// Default protection flag (not used in Windows)
#define _MERRY_PROT_DEFAULT_

// Default flag (not used in Windows)
#define _MERRY_FLAG_DEFAULT_
#endif

typedef mptr_t mdlentry_t; // same for windows and linux

_MERRY_DEFINE_FUNC_PTR_(_THRET_T_, mthexec_t, mptr_t);

mret_t merry_open_pipe(mdataline_t *rline, mdataline_t *wline);

#endif