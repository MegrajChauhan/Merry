#include <merry_twc.h>

mret_t merry_twc_base_form(MerryTWCBase *base)
{
    merry_check_ptr(base);
    if (merry_open_pipe(&base->lines._read_line_for_child, &base->lines._write_line_for_parent) != RET_FAILURE)
        return RET_FAILURE;
    if (merry_open_pipe(&base->lines._read_line_for_parent, &base->lines._write_line_for_child) != RET_FAILURE)
    {
        close(base->lines._read_line_for_child);
        close(base->lines._write_line_for_parent);
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

MerryTWC *merry_twc_create(mdataline_t rline, mdataline_t wline)
{
    MerryTWC *twc = (MerryTWC *)malloc(sizeof(MerryTWC));
    if (!twc)
        return RET_NULL;

    merry_owc_init(&twc->rc, rline, 0, mfalse, mtrue);
    merry_owc_init(&twc->wc, 0, wline, mtrue, mfalse);

    return twc;
}

mret_t merry_twc_send(MerryTWC *twc, mbptr_t data, msize_t len)
{
    merry_check_ptr(twc);
    merry_check_ptr(data);

    if (surelyF(len == 0))
        return RET_SUCCESS;

    return merry_owc_speak(&twc->wc, data, len);
}

mret_t merry_twc_receive(MerryTWC *twc, mbptr_t buf, msize_t n)
{
    merry_check_ptr(twc);
    merry_check_ptr(buf);

    if (surelyF(n == 0))
        return RET_SUCCESS;

    return merry_owc_listen(&twc->rc, buf, n);
}

void merry_twc_destroy(MerryTWC *twc)
{
    merry_check_ptr(twc);
    merry_owc_only_listen(&twc->wc);
    merry_owc_only_speak(&twc->rc);
    free(twc);
}