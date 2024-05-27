#ifndef _MERRY_ERRNO_
#define _MERRY_ERRNO_

#include <errno.h>

#define _MERRY_FD_IN_BAD_STATE_ 45 /* File descriptor in a bad state */
#define _MERRY_NO_PERM_ 46         /* No permission to perform the task */

#ifdef _MERRY_HOST_OS_LINUX_

#elif defined(_MERRY_HOST_OS_WINDOWS_)

#endif

#endif