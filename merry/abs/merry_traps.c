#include "merry_traps.h"

mret_t merry_trap_install()
{
#ifdef _USE_LINUX_
    struct sigaction sa = {0};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = &merry_handle_termination;
    if (sigaction(SIGTERM, &sa, NULL) == -1)
        return RET_FAILURE;
    sa.sa_handler = &merry_handle_interrupt;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        return RET_FAILURE;
    sa.sa_handler = &merry_handle_segv;
    if (sigaction(SIGSEGV, &sa, NULL) == -1)
        return RET_FAILURE;
#endif
    return RET_SUCCESS;
}