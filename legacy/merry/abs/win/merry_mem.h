#ifndef _MERRY_MEM_
#define _MERRY_MEM_
#include <windows.h>

// Memory allocation
#define _MERRY_MEM_GET_PAGE_(size, prot, flags) VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
#define _MERRY_MEM_GIVE_PAGE_(addr, size) VirtualFree(addr, 0, MEM_RELEASE)
#define _MERRY_RET_GET_ERROR_ NULL
#define _MERRY_RET_GIVE_ERROR_ 0

// Default protection flag (not used in Windows)
#define _MERRY_PROT_DEFAULT_

// Default flag (not used in Windows)
#define _MERRY_FLAG_DEFAULT_

// No support for extending system break point in Windows
#define _MERRY_MEM_BRK_SUPPORT_ 0
#define _MERRY_MEM_GET_CURRENT_BRK_POINT_ NULL
#define _MERRY_MEM_INCREASE_BRK_POINT_(len) NULL
#define _MERRY_MEM_BRK_ERROR_ NULL

#endif