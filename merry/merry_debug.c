#include "merry_debug.h"

MerryDebug *merry_init_debug()
{
    MerryCond *c = merry_cond_init();
    MerryMutex *l = merry_mutex_init();
    if (c == RET_NULL || l == RET_NULL)
        goto _err;
    char finalf[128] = {0};
    finalf[0] = 'i';
    finalf[1] = 'n';
    finalf[2] = 'a';
    msize_t i = 2;
    while (mtrue)
    {
        FILE *f = fopen(finalf, "r");
        if (f == NULL)
        {
            if (finalf[i] != 'z')
                finalf[i]++;
            else
            {
                i++;
                finalf[i] = 'a';
            }
        }
        else
        {
            fclose(f);
            break;
        }
    }
#ifdef _USE_LINUX_
    int ih = open(finalf, O_RDWR | O_CREAT, S_IRWXU);
    if (ih == -1)
        goto _err;
    memcpy((finalf + 2), finalf + 3, sizeof(finalf));
    finalf[0] = 'o';
    finalf[1] = 'u';
    finalf[2] = 't';
    int oh = open(finalf, O_RDWR | O_CREAT, S_IRWXU);
    if (oh == -1)
    {
        close(ih);
        goto _err;
    }
#endif
    MerryDebug *dbg = (MerryDebug *)malloc(sizeof(MerryDebug));
    if (dbg == NULL)
    {
#ifdef _USE_LINUX_
        close(ih);
        close(oh);
#endif
        goto _err;
    }
    dbg->cond = c;
    dbg->lock = l;
    dbg->input_handle = ih;
    dbg->output_handle = oh;
    dbg->stop = mfalse;
    return dbg;
_err:
    merry_cond_destroy(c);
    merry_mutex_destroy(l);
    return RET_NULL;
}

void merry_destroy_debug(MerryDebug *dbg)
{
    if (dbg == NULL)
        return;
    merry_cond_destroy(dbg->cond);
    merry_mutex_destroy(dbg->lock);
#ifdef _USE_LINUX_
    close(dbg->input_handle);
    close(dbg->output_handle);
#endif
    free(dbg);
}

_THRET_T_ merry_start_debugging(mptr_t arg)
{
    MerryDebug *dbg = (MerryDebug *)arg;
#ifdef _USE_LINUX_
    int epoll_fd, fc;
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        goto _err;
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = dbg->input_handle;
    if ((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, dbg->input_handle, &ev)) == -1)
        goto _err;
    while (dbg->stop == mfalse)
    {
        struct epoll_event e;
        fc = epoll_wait(epoll_fd, &e, 1, -1);
        if (fc == -1)
            goto _err;
        // How the commands are transmitted?
        // A total of 16 bytes each time is read each time.
        // The first 8 bytes indicate the request for the VM
        // while the second byte indicates the operand for the request
        // In future, the input length may be variable based on the request
        mbyte_t buf[16];
        if (read(e.data.fd, buf, 16) < 16)
            fprintf(stderr, "INVALID DEBUG COMMAND[WARNING]\n");
        else
        {
            register mqword_t op = *(mqptr_t)buf;
            switch (op)
            {
            case _DBG_CLOSED_:
                // no need to do anything
                // This is the point where the debugger will fulfill every command sent to it
                // and will accept no more
                merry_os_dbg_stopped(); // stop taking any more requests
                while (read(e.data.fd, buf, 16) > 0)
                {
                    merry_requestHdlr_push_request(*(mqptr_t)buf, *(mqptr_t)(buf + 8), NULL);
                }
                atomic_store(&dbg->stop, mtrue);
                continue;
            }
            merry_requestHdlr_push_request(op, *(mqptr_t)(buf + 8), NULL);
        }
    }
    return RET_NULL;
_err:
    merry_requestHdlr_push_request(MERRY_DBG_UNAVILABLE, 0, dbg->cond);
    return RET_NULL;
#endif
}

void merry_send_to_debugger(MerryDebug *dbg, mqword_t op, mqword_t oper)
{
    mqword_t buf[2] = {op, oper};
    write(dbg->output_handle, (mbptr_t)buf, 16);
}