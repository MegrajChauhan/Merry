#include "merry_traps.h"

#ifdef _USE_WIN_
BOOL WINAPI merry_handle_ctrl_event(DWORD ctrlType)
{
    switch (ctrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        merry_handle_interrupt();
        return TRUE;
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        merry_handle_termination();
        return TRUE;
    default:
        return FALSE;
    }
}

LONG WINAPI merry_handle_exception(EXCEPTION_POINTERS *ExceptionInfo)
{
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    {
        merry_handle_segv(); // Handle segmentation fault (access violation)
        RemoveVectoredExceptionHandler(handle);
        return EXCEPTION_EXECUTE_HANDLER;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

mret_t merry_trap_install()
{
    inlog("Installing Trap Handlers");
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
#else
    if (!SetConsoleCtrlHandler(merry_handle_ctrl_event, TRUE))
        return RET_FAILURE;
    handle = AddVectoredExceptionHandler(1, merry_handle_exception);
#endif
    inlog("Trap Handlers successfully installed.");
    return RET_SUCCESS;
}