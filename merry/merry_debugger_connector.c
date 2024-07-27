#include "merry_debugger_connector.h"

MerryDBSupp *merry_init_dbsupp()
{
    MerryDBSupp *dbg = (MerryDBSupp *)malloc(sizeof(MerryDBSupp));
    if (dbg == NULL)
        return RET_NULL;
#ifdef _USE_LINUX_
    dbg->_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (dbg->_fd == -1)
        free(dbg);
    dbg->_addr.sin_family = AF_INET;
    dbg->_addr.sin_addr.s_addr = INADDR_ANY;
    dbg->_addr.sin_port = _MERRY_DEFAULT_PORT_;
#endif
    if ((dbg->cond = merry_cond_init()) == RET_NULL)
        merry_destroy_dbsupp(dbg);
    else if ((dbg->lock = merry_mutex_init()) == RET_NULL)
        merry_destroy_dbsupp(dbg);
    return dbg;
}

void merry_destroy_dbsupp(MerryDBSupp *dbg)
{
#ifdef _USE_LINUX_
    close(dbg->_fd);
    close(dbg->_listen_fd);
#endif
    merry_cond_destroy(dbg->cond);
    merry_mutex_destroy(dbg->lock);
    free(dbg);
    dbg = NULL;
}

_THRET_T_ merry_dbg_run(mptr_t _ptr)
{
    MerryDBSupp *dbg = (MerryDBSupp *)_ptr;
#ifdef _USE_LINUX_
    if (bind(dbg->_fd, (struct sockaddr *)&dbg->_addr, sizeof(dbg->_addr)) < 0)
    {
        perror("DEBUGGER CONNECT");
        return NULL;
    }
    if (listen(dbg->_fd, 1) < 0)
    {
        perror("DEBUUGER LISTEN");
        return NULL;
    }
    socklen_t addrlen = sizeof(dbg->_addr);
    if ((dbg->_listen_fd = accept(dbg->_fd, (struct sockaddr *)&dbg->_addr, &addrlen)) < 0)
    {
        perror("DEBUGGER ACCEPT");
        return NULL;
    }

#endif
    ssize_t read_into_buf;
    while (mtrue)
    {
        merry_mutex_lock(dbg->lock);
        if (atomic_load(&dbg->_send_sig) == mtrue)
        {
#ifdef _USE_LINUX_
            send(dbg->_listen_fd, dbg->sig, _MERRY_PER_EXCG_BUF_LEN_, 0);
#endif
            if (dbg->sig[0] == _TERMINATING_)
                break;
            continue;
        }
#ifdef _USE_LINUX_
        read_into_buf = read(dbg->_listen_fd, dbg->sig, _MERRY_PER_EXCG_BUF_LEN_);
#endif
        if (read_into_buf == 0)
            continue;
        merry_mutex_unlock(dbg->lock);
        merry_requestHdlr_push_dbg_request(_REQ_INTR, dbg->cond, dbg->sig);
    }
#ifdef _USE_LINUX_
    close(dbg->_listen_fd);
    return NULL;
#endif
    return 0;
}