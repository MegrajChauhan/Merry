/*
 * MIT License
 *
 * Copyright (c) 2024 MegrajChauhan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _MERRY_DBG_
#define _MERRY_DBG_

#include "merry_config.h"
#include "merry_types.h"
#include "merry_utils.h"
#include "merry_request_hdlr.h"
#include "merry_console.h"
#include "merry_commands.h"
#include "merry_queue.h"
#include <stdatomic.h>
#include <string.h>

#ifdef _USE_LINUX_
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#define _MERRY_DEFAULT_LISTEN_PORT_ 4048
#define _MERRY_DEFAULT_SEND_PORT_ 4144
#define _MERRY_PER_EXCG_BUF_LEN_ 16

typedef struct MerryListener MerryListener;
typedef struct MerrySender MerrySender;
typedef struct MerrySendQueue MerrySendQueue;
typedef struct MerrySendQueueNode MerrySendQueueNode;
typedef struct MerrySigNode MerrySigNode;

struct MerrySigNode
{
    mbyte_t sig[_MERRY_PER_EXCG_BUF_LEN_];
};

_MERRY_CREATE_QUEUE_NODE_NOPTR_(MerrySigNode, MerrySendQueueNode);
_MERRY_CREATE_QUEUE_(MerrySendQueue, MerrySendQueueNode);

// We could stop the Listener is a very clean way but once the OS terminates, the Listener
// We need a convention here too.
// The thread could wait indefinitely for an input but the connected program could very
// well just terminate. So we need the program to send in a signal after receiving _TERMINATING_
// so that the Listener thread terminates peacefully.
struct MerryListener
{
#ifdef _USE_LINUX_
    int _fd;
    int _listen_fd;
    struct sockaddr_in _addr;
#endif
    mbyte_t sig[_MERRY_PER_EXCG_BUF_LEN_];
    mbool_t notify_os;
    mbool_t stop;
};

struct MerrySender
{
#ifdef _USE_LINUX_
    int _send_fd;
    struct sockaddr_in _addr;
#endif
    MerrySendQueue *queue;
    MerryCond *cond;
    MerryMutex *lock;
    mbool_t notify_os;
    mbool_t stop;
};

extern void merry_os_notice(mbool_t _type);

MerryListener *merry_init_listener(mbool_t notify_os);

MerrySender *merry_init_sender(mbool_t notify_os);

void merry_destroy_listener(MerryListener *dbg);

void merry_destroy_sender(MerrySender *dbg);

void merry_cleanup_sender(MerrySender *dbg);

mret_t merry_sender_push_sig(MerrySender *sender, mbptr_t sig); // used by the OS

mret_t merry_sender_pop_sig(MerrySender *sender, MerrySendQueueNode *node); // used by the sender

_THRET_T_ merry_start_listening(mptr_t _list);

_THRET_T_ merry_start_sending(mptr_t _send);

#endif