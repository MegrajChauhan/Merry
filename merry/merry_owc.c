#include <merry_owc.h>

_MERRY_INTERNAL_ mret_t merry_open_pipe_owc(MerryOWC *owc)
{
    merry_check_ptr(owc);
    return merry_open_pipe(&owc->_read_line, &owc->_write_line);
}

MerryOWC *merry_open_owc()
{
    MerryOWC *owc = (MerryOWC *)malloc(sizeof(MerryOWC));
    if (owc == NULL)
        return RET_NULL;

    if (merry_open_pipe_owc(owc) == RET_FAILURE)
    {
        free(owc);
        return RET_NULL;
    }

    /// NOTE: The _in_use flags refers to the state where at least one line is still open.
    /// NOTE: If both lines are closed, then the OWC is free. i.e _rclosed and _wclosed must be mtrue
    owc->_in_use = mtrue;
    owc->_rclosed = mfalse;
    owc->_wclosed = mfalse;
    return owc;
}

void merry_owc_only_speak(MerryOWC *owc)
{
    merry_check_ptr(owc);

// The owner of the channel only wants to speak via the channel
#ifdef _USE_LINUX_
    if (owc->_rclosed == mfalse)
        close(owc->_read_line);
    owc->_rclosed = mtrue;
#elif defined(_USE_WIN_)
    if (!owc->_rclosed && owc->_read_line != INVALID_HANDLE_VALUE)
        CloseHandle(owc->_read_line);
    owc->_rclosed = mtrue;
#endif
}

void merry_owc_only_listen(MerryOWC *owc)
{
    merry_check_ptr(owc);

// The owner of the channel only wants to listen via the channel
#ifdef _USE_LINUX_
    if (owc->_wclosed == mfalse)
        close(owc->_write_line);
    owc->_wclosed = mtrue;
#elif defined(_USE_WIN_)
    if (!owc->_wclosed && owc->_write_line != INVALID_HANDLE_VALUE)
        CloseHandle(owc->_write_line);
    owc->_wclosed = mtrue;
#endif
}

void merry_owc_free_channel(MerryOWC *owc)
{
    merry_owc_only_listen(owc);
    merry_owc_only_speak(owc);
    owc->_in_use = mfalse;
}

mret_t merry_owc_reopen(MerryOWC *owc)
{
    merry_check_ptr(owc);

    // to be sure, we will make sure to close both ends
    merry_owc_free_channel(owc);
    if (merry_open_pipe_owc(owc) == RET_FAILURE)
        return RET_FAILURE;

    owc->_in_use = mtrue;
    owc->_rclosed = mfalse;
    owc->_wclosed = mfalse;

    return RET_SUCCESS;
}

void merry_destroy_owc(MerryOWC *owc)
{
    merry_check_ptr(owc);
    merry_owc_free_channel(owc);
    free(owc);
}

mret_t merry_owc_speak(MerryOWC *owc, mbptr_t data, msize_t len)
{
    // The OWC will speak for you
    // Send len bytes
    merry_check_ptr(owc);
    merry_check_ptr(data);
    if (surelyF(!len))
        return RET_SUCCESS;

    if (surelyF(owc->_wclosed))
        return RET_FAILURE; // maybe the channel was configured to be a listener for this owner

    if (write(owc->_write_line, (mptr_t)data, len) == -1)
        return RET_UNKNWON;

    return RET_SUCCESS;
}

mret_t merry_owc_listen(MerryOWC *owc, mbptr_t buf, msize_t n)
{
    // The OWC will listen for you
    // Send len bytes
    merry_check_ptr(owc);
    merry_check_ptr(buf);
    if (surelyF(!n))
        return RET_SUCCESS;
    if (surelyF(owc->_rclosed))
        return RET_FAILURE; // maybe the channel was configured to be a speaker for this owner

    if (read(owc->_read_line, (mptr_t)buf, n) == -1)
        return RET_UNKNWON;

    return RET_SUCCESS;
}