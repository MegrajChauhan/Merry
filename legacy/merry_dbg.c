#include "merry_dbg.h"

MerryListener *merry_init_listener(mbool_t notify_os, msize_t port_num)
{
    MerryListener *dbg = (MerryListener *)malloc(sizeof(MerryListener));
    if (dbg == NULL)
        return RET_NULL;
#ifdef _USE_LINUX_
    dbg->_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (dbg->_fd == -1)
    {
        free(dbg);
        return NULL;
    }
    dbg->_addr.sin_family = AF_INET;
    dbg->_addr.sin_addr.s_addr = INADDR_ANY;
    dbg->_addr.sin_port = port_num;
#elif defined(_USE_WIN_)
    WSADATA _data;
    WSAStartup(MAKEWORD(2, 2), &_data);
    dbg->_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dbg->_fd == INVALID_SOCKET)
    {
        free(dbg);
        return;
    }
    dbg->_addr.sin_family = AF_INET;
    dbg->_addr.sin_addr.s_addr = INADDR_ANY;
    dbg->_addr.sin_port = port_num;
#endif
    dbg->notify_os = notify_os;
    dbg->stop = mfalse;
    dbg->t1 = dbg->t2 = 0;
    return dbg;
}

MerrySender *merry_init_sender(mbool_t notify_os, msize_t port_num)
{
    MerrySender *dbg = (MerrySender *)malloc(sizeof(MerrySender));
    if (dbg == NULL)
        return RET_NULL;
#ifdef _USE_LINUX_
    dbg->_send_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (dbg->_send_fd == -1)
        free(dbg);
    dbg->_addr.sin_family = AF_INET;
    dbg->_addr.sin_addr.s_addr = INADDR_ANY;
    dbg->_addr.sin_port = port_num;
#elif defined(_USE_WIN_)
    dbg->_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dbg->_fd == INVALID_SOCKET)
    {
        free(dbg);
        return;
    }
    dbg->_addr.sin_family = AF_INET;
    dbg->_addr.sin_addr.s_addr = INADDR_ANY;
    dbg->_addr.sin_port = port_num;

#endif
    if ((dbg->cond = merry_cond_init()) == RET_NULL)
        goto err;
    if ((dbg->lock = merry_mutex_init()) == RET_NULL)
        goto err;
    _MERRY_QUEUE_INIT_(MerrySendQueue, dbg->queue);
    if (dbg->queue == NULL)
        goto err;
    mbool_t t;
    _MERRY_QUEUE_CREATE_NODES_NOPTR_(dbg->queue, MerrySendQueueNode, 10, t);
    if (t == mfalse)
        goto err;
    dbg->notify_os = notify_os;
    dbg->stop = mfalse;
    return dbg;
err:
    merry_destroy_sender(dbg);
    return RET_NULL;
}

void merry_destroy_listener(MerryListener *dbg)
{
    if (dbg == NULL)
        return;
#ifdef _USE_LINUX_
    close(dbg->_fd);
    close(dbg->_listen_fd);
#elif defined(_USE_WIN_)
    WSACleanup();
    closesocket(dbg->_fd);
    closesocket(dbg->_listen_fd);
#endif
    free(dbg);
}

void merry_destroy_sender(MerrySender *dbg)
{
    if (dbg == NULL)
        return;
    merry_cond_destroy(dbg->cond);
    merry_mutex_destroy(dbg->lock);
    _MERRY_DESTROY_QUEUE_NOPTR_(dbg->queue);
#ifdef _USE_LINUX_
    close(dbg->_send_fd);
#elif defined(_USE_WIN_)
    closesocket(dbg->_send_fd);
#endif
}

/*Un-used and deprecated*/
void merry_cleanup_sender(MerrySender *dbg)
{
    if (dbg == NULL)
        return;
    merry_mutex_destroy(dbg->lock);
    _MERRY_DESTROY_QUEUE_NOPTR_(dbg->queue);
#ifdef _USE_LINUX_
    close(dbg->_send_fd);
#endif
}

_THRET_T_ merry_start_listening(mptr_t _list)
{
    MerryListener *dbg = (MerryListener *)_list;
#ifdef _USE_LINUX_
    if (bind(dbg->_fd, (struct sockaddr *)&dbg->_addr, sizeof(dbg->_addr)) < 0)
    {
        perror("DEBUGGER CONNECT");
        merry_os_notice(3);
        return NULL;
    }
    if (listen(dbg->_fd, 5) < 0)
    {
        perror("DEBUUGER LISTEN");
        merry_os_notice(3);
        return NULL;
    }
    socklen_t addrlen = sizeof(dbg->_addr);
    if ((dbg->_listen_fd = accept(dbg->_fd, (struct sockaddr *)&dbg->_addr, &addrlen)) < 0)
    {
        perror("DEBUGGER ACCEPT");
        merry_os_notice(3);
        return NULL;
    }
#elif defined(_USE_LINUX_)
    int addrlen = sizeof(dbg->_addr);
    if (bind(dbg->_fd, (struct sockaddr *)&dbg->_addr, addrlen) == SOCKET_ERROR)
    {
        printf("DEBUGGER CONNECT failed with error: %d\n", WSAGetLastError());
        merry_os_notice(3);
        return NULL;
    }
    if (listen(dbg->_fd, 5) == SOCKET_ERROR)
    {
        printf("DEBUGGER LISTEN failed with error: %d\n", WSAGetLastError());
        merry_os_notice(3);
        return NULL;
    }
    if ((dbg->_listen_fd = accept(dbg->_fd, (struct sockaddr *)&dbg->_addr, &addrlen)) == INVALID_SOCKET)
    {
        printf("DEBUGGER ACCEPT failed with error: %d\n", WSAGetLastError());
        merry_os_notice(3);
        return NULL;
    }
#endif
    if (dbg->notify_os == mtrue)
        merry_os_notice(mtrue);
    else
        merry_requestHdlr_push_request(_REQ_GDB_INIT, 0, NULL);
    ssize_t read_into_buf;
    while (mtrue)
    {
#ifdef _USE_LINUX_
        read_into_buf = read(dbg->_listen_fd, dbg->sig, _MERRY_PER_EXCG_BUF_LEN_);
#elif defined(_USE_WIN_)
        read_into_buf = recv(dbg->_listen_fd, dbg->sig, _MERRY_PER_EXCG_BUF_LEN_, 0);
#endif
        if (dbg->stop == mtrue)
            break;
        if (read_into_buf == 0)
            continue;
        // merry_requestHdlr_push_request_dbg(_REQ_INTR);
        mbyte_t op = dbg->sig[0], arg_id;
        maddress_t addr;
        switch (dbg->sig[0])
        {
        case _GET_CORE_COUNT_:
        case _GET_OS_ID_:
        case _GET_DATA_MEM_PAGE_COUNT_:
        case _GET_INST_MEM_PAGE_COUNT_:
            break;
        case _INST_AT_:
        case _DATA_AT_:
        case _ADD_BREAKPOINT_:
            addr = dbg->sig[8];
            (addr <<= 8);
            addr |= dbg->sig[9];
            (addr <<= 8);
            addr |= dbg->sig[10];
            (addr <<= 8);
            addr |= dbg->sig[11];
            (addr <<= 8);
            addr |= dbg->sig[12];
            (addr <<= 8);
            addr |= dbg->sig[13];
            (addr <<= 8);
            addr |= dbg->sig[14];
            (addr <<= 8);
            addr |= dbg->sig[15];
            break;
        case _SP_OF_:
        case _BP_OF_:
        case _PC_OF_:
        case _CONTINUE_CORE_:
            arg_id = dbg->sig[15];
            break;
        case _REGR_OF_:
            arg_id = dbg->sig[15];
            addr = dbg->sig[14] & 15;
            break;
        case _IO_PORT_NUM_:
        {
            msize_t ip, op;
            ip = dbg->sig[1];
            ip <<= 8;
            ip |= dbg->sig[2];
            op = dbg->sig[3];
            op <<= 8;
            op |= dbg->sig[4];
            atomic_exchange(&dbg->t1, ip);
            atomic_exchange(&dbg->t2, op);
            continue;
        }
        }
        merry_requestHdlr_push_request_dbg(_REQ_INTR, op, arg_id, addr);
    }
    merry_os_notice(3);
#ifdef _USE_LINUX_
    return NULL;
#endif
    return 0;
}

_THRET_T_ merry_start_sending(mptr_t _send)
{
    MerrySender *dbg = (MerrySender *)_send;
#ifdef _USE_LINUX_
    for (msize_t i = 0; i <= 1000; i++)
    {
        if (i == 999)
        {
            merry_os_notice(2);
            return NULL;
        }
        if (connect(dbg->_send_fd, (struct sockaddr *)&dbg->_addr, sizeof(dbg->_addr)) < 0)
            usleep(10);
        else
            break;
    }
    // we keep trying until we connect but only for 10000 times
#elif defined(_USE_WIN_)
    for (msize_t i = 0; i <= 1000; i++)
    {
        if (i == 999)
        {
            merry_os_notice(2);
            return NULL;
        }
        if (connect(dbg->_send_fd, (struct sockaddr *)&dbg->_addr, sizeof(dbg->_addr)) == SOCKET_ERROR)
            Sleep(1); // sleep for 1 millisecond but keep trying
        else
            break;
    }
#endif
    if (dbg->notify_os == mtrue)
        merry_os_notice(mfalse);
    else
        merry_requestHdlr_push_request(_REQ_GDB_INIT, 1, NULL);
    MerrySendQueueNode node;
    while (mtrue)
    {
        merry_mutex_lock(dbg->lock);
        if (merry_sender_pop_sig(dbg, &node) == mfalse)
            merry_cond_wait(dbg->cond, dbg->lock);
        if (dbg->stop == mtrue)
        {
            if (dbg->queue->data_count > 0)
            {
                while (merry_sender_pop_sig(dbg, &node) != mfalse)
                {
                    send(dbg->_send_fd, node.value.sig, _MERRY_PER_EXCG_BUF_LEN_, 0);
                }
            }
            // printf("Sender stopped.\n");
            break;
        }
        merry_sender_pop_sig(dbg, &node); // this should work
        send(dbg->_send_fd, node.value.sig, _MERRY_PER_EXCG_BUF_LEN_, 0);
        merry_mutex_unlock(dbg->lock);
    }
    merry_os_notice(2);
#ifdef _USE_LINUX_
    return NULL;
#endif
    return 0;
}

mret_t merry_sender_push_sig(MerrySender *sender, mbptr_t sig)
{
    merry_mutex_lock(sender->lock);
    MerrySigNode node;
    memcpy(node.sig, sig, _MERRY_PER_EXCG_BUF_LEN_);
    mbool_t ret;
    _MERRY_QUEUE_PUSH_NOPTR_(sender->queue, node, ret);
    if (ret == mfalse)
        goto _exit_;
    if (sender->queue->data_count == 1)
        merry_cond_signal(sender->cond);
    goto _exit_;
_exit_:
    merry_mutex_unlock(sender->lock);
    return ret;
}

mret_t merry_sender_pop_sig(MerrySender *sender, MerrySendQueueNode *node)
{
    mbool_t ret;
    _MERRY_QUEUE_POP_NOPTR_(sender->queue, node->value, ret);
    goto _exit_;
_exit_:
    return ret;
}