#include "merry_config.h"
#include "merry_types.h"
#include "merry_dynl.h"
#include "merry_request.h"
#include "merry_thread.h"
#include <stdlib.h>
#include <string.h>
#ifdef _USE_LINUX_
#include <unistd.h>
#endif

static void merry_subsys_send_back(int _wfd, int req, mqword_t val)
{
    mqword_t buf[2];
    buf[0] = req;
    buf[1] = val;
    write(_wfd, (mptr_t)buf, 16);
}

// The arguments: The write end of the pipe to send, id, read end of the pipe to receive and the path to the subsystem
int main(int argc, char **argv)
{
    int ret = 0;
#ifdef _USE_LINUX_
    int _wfd = strtol(argv[1], NULL, 10);
    int _rfd = strtol(argv[2], NULL, 10);
    mptr_t handle;
#endif
    msize_t id = strtol(argv[3], NULL, 10);
    mstr_t subsysname = argv[4];
    if (merry_loadLib(subsysname, &handle) == mfalse)
    {
        merry_subsys_send_back(_wfd, _SUBSYS_FAILED, 0);
        return RET_FAILURE;
    }
    subsys_t subsys_main = merry_libsym(handle, subsysname);
    if (subsys_main == RET_NULL)
    {
        merry_unloadLib(&handle);
        merry_subsys_send_back(_wfd, _SUBSYS_FAILED, 0);
        return RET_FAILURE;
    }
    ret = subsys_main(_rfd, _wfd, id);
    merry_unloadLib(&handle);
    return ret;
}