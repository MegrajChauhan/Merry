#include "merry_process.h"

mbool_t merry_create_process(MerryProcess *p)
{
    msize_t _e;
#ifdef _USE_LINUX_
    p->pid = fork();
    if (p->pid == -1)
    {
        _e = MERRY_SYSERR;
        goto _err;
    }
#elif defined(_USE_WIN_) 
    /// TODO: FIX ME
    msize_t _opt_len = 0;
    mstr_t *_opt = NULL;
    merry_get_cmd_options(&_opt_len, &_opt);
    if (*_opt == NULL)
    {
        _e = MERRY_VMERR;
        goto _err;
    }
    ZeroMemory(&p->si, sizeof(p->si));
    p->si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcess(
            NULL,               // No module name (use command line)
            _opt,               // Command line
            NULL,               // Process handle not inheritable
            NULL,               // Thread handle not inheritable
            FALSE,              // Set handle inheritance to FALSE
            CREATE_NEW_CONSOLE, // Create a new console window for the process
            NULL,               // Use parent's environment block
            NULL,               // Use parent's starting directory
            &si,                // Pointer to STARTUPINFO structure
            &pi)                // Pointer to PROCESS_INFORMATION structure
    )
    {
        _e = MERRY_SYSERR;
        goto _err;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif
    return mtrue;
_err:
    merry_set_errno(_e);
    return mfalse;
}

mpid_t merry_get_pid()
{
    return getpid();
}