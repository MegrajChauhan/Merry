#include <merry_platform.h>

mret_t merry_open_pipe(mdataline_t *rline, mdataline_t *wline)
{
    merry_check_ptr(rline);
    merry_check_ptr(wline);

    mdataline_t lines[2];
// #ifdef _USE_LINUX_
//     if (pipe(owc->pfd) == -1)
//         return RET_FAILURE;
// #elif defined(_USE_WIN_)
// #endif
//     return RET_SUCCESS;
#ifdef _USE_LINUX_
    if (pipe(lines) == -1)
        return RET_FAILURE;
#elif defined(_USE_WIN_)
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE; // Handles can be inherited by child processes
    saAttr.lpSecurityDescriptor = NULL;

    // Create an anonymous owc
    if (!CreatePipe(&lines[0], &lines[1], &saAttr, 0)) // Default size
        return RET_FAILURE;
#endif
    *rline = lines[0];
    *wline = lines[1];
    return RET_SUCCESS;
}