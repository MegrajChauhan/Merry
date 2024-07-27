#ifndef _MERRY_DBG_SUPP_
#define _MERRY_DBG_SUPP_

#include "merry_config.h"
#include "merry_types.h"
#include "merry_utils.h"
#include "merry_request_hdlr.h"
#include "merry_console.h"
#include "merry_commands.h"
#include <stdatomic.h>

#ifdef _USE_LINUX_
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#define _MERRY_DEFAULT_PORT_ 55000

// The debugger should expect 16 byte
#define _MERRY_PER_EXCG_BUF_LEN_ 16

typedef struct MerryDBSupp MerryDBSupp;

struct MerryDBSupp
{
#ifdef _USE_LINUX_
    int _fd;
    int _listen_fd;
    struct sockaddr_in _addr;
#endif
    mbyte_t sig[_MERRY_PER_EXCG_BUF_LEN_];
    mbool_t _send_sig; // The Manager thread sets this when some important event takes place(new core creation, process creation, syscall etc).
                       // 'sig' will be set to an appropriate value which is sent to the debugger.
                       // The debugger can interpret the 'sig' and send response similarly
    MerryCond *cond;
    MerryMutex *lock;
};

MerryDBSupp *merry_init_dbsupp();

void merry_destroy_dbsupp(MerryDBSupp *dbg);

_THRET_T_ merry_dbg_run(mptr_t _ptr);

#endif