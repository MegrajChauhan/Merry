#include "merry_pipe.h"

MerryPipe *merry_open_pipe()
{
    MerryPipe *p = (MerryPipe *)malloc(sizeof(MerryPipe));
    if (p == NULL)
        return RET_NULL;

#ifdef _USE_LINUX_
    if (pipe(p->pfd) == -1)
    {
        free(p);
        return RET_NULL;
    }
#elif defined(_USE_WIN_)
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE; // Handles can be inherited by child processes
    saAttr.lpSecurityDescriptor = NULL;

    // Create an anonymous pipe
    if (!CreatePipe(&p->_read_handle, &p->_write_handle, &saAttr, 0)) // Default size
    {
        free(p);
        return RET_NULL;
    }

#endif
    p->_in_use = mtrue;
    p->_rclosed = mfalse;
    p->_wclosed = mfalse;
    return p;
}

void merry_pipe_close_one_end(MerryPipe *pipe, int end)
{
    if (surelyF(pipe == RET_NULL))
        return;

    switch (end)
    {
#ifdef _USE_LINUX_
    case _MERRY_CLOREND_:
        if (pipe->_rclosed == mfalse)
            close(pipe->_read_fd);
        pipe->_rclosed = mtrue;
        break;
    case _MERRY_CLOWEND_:
        if (pipe->_wclosed == mfalse)
            close(pipe->_write_fd);
        pipe->_wclosed = mtrue;
        break;
#elif defined(_USE_WIN_)
    case _MERRY_CLOREND_:
        if (!pipe->_rclosed && pipe->_read_handle != INVALID_HANDLE_VALUE)
            CloseHandle(pipe->_read_handle);
        pipe->_rclosed = mtrue;
        break;
    case _MERRY_CLOWEND_:
        if (!pipe->_wclosed && pipe->_write_handle != INVALID_HANDLE_VALUE)
            CloseHandle(pipe->_write_handle);
        pipe->_wclosed = mtrue;
        break;
#endif
    }
}

void merry_pipe_close_both_ends(MerryPipe *pipe)
{
    if (surelyF(pipe == RET_NULL))
        return;

#ifdef _USE_LINUX_
    if (pipe->_rclosed == mfalse)
        close(pipe->_read_fd);
    pipe->_rclosed = mtrue;
    if (pipe->_wclosed == mfalse)
        close(pipe->_write_fd);
    pipe->_wclosed = mtrue;
#elif defined(_USE_WIN_)
    if (!pipe->_rclosed && pipe->_read_handle != INVALID_HANDLE_VALUE)
        CloseHandle(pipe->_read_handle);
    pipe->_rclosed = mtrue;
    if (!pipe->_wclosed && pipe->_write_handle != INVALID_HANDLE_VALUE)
        CloseHandle(pipe->_write_handle);
    pipe->_wclosed = mtrue;
#endif
    pipe->_in_use = mfalse;
}

mret_t merry_pipe_reopen(MerryPipe *_pipe)
{
    if (surelyF(_pipe == RET_NULL))
        return;

#ifdef _USE_LINUX_
    if (pipe(_pipe->pfd) == -1)
        return RET_FAILURE;
#elif defined(_USE_WIN_)
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE; // Handles can be inherited by child processes
    saAttr.lpSecurityDescriptor = NULL;

    // Create an anonymous pipe
    if (!CreatePipe(&_pipe->_read_handle, &_pipe->_write_handle, &saAttr, 0)) // Default size
        return RET_FAILURE;

#endif
    _pipe->_in_use = mtrue;
    _pipe->_rclosed = mfalse;
    _pipe->_wclosed = mfalse;
    return RET_SUCCESS;
}

void merry_destroy_pipe(MerryPipe *pipe)
{
    if (surelyF(pipe == RET_NULL))
        return;

#ifdef _USE_LINUX_
    if (pipe->_rclosed != mtrue)
        close(pipe->_read_fd);
    if (pipe->_wclosed != mtrue)
        close(pipe->_write_fd);
#elif defined(_USE_WIN_)
    if (!pipe->_rclosed && pipe->_read_handle != INVALID_HANDLE_VALUE)
        CloseHandle(pipe->_read_handle);
    if (!pipe->_wclosed && pipe->_write_handle != INVALID_HANDLE_VALUE)
        CloseHandle(pipe->_write_handle);
#endif
    free(pipe);
}

#ifdef _USE_WIN_
void merry_pipe_prevent_inheritance(MerryPipe *pipe, int flag)
{
    if (surelyF(pipe == RET_NULL))
        return;
    switch (flag)
    {
    case _MERRY_IN_RFD_:
        SetHandleInformation(pipe->_read_handle, HANDLE_FLAG_INHERIT, 0);
        break;
    case _MERRY_IN_WFD_:
        SetHandleInformation(pipe->_write_handle, HANDLE_FLAG_INHERIT, 0);
        break;
    }
}
#endif