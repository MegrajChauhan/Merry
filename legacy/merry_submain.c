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
    log("Sub-System Reply: %sREQ=%d, VALUE=%lu%s", BOLDGREEN, req, val, RESET);
    write(_wfd, (mptr_t)buf, 16);
}

// The arguments: The write end of the pipe to send, id, read end of the pipe to receive and the path to the subsystem
int main(int argc, char **argv)
{
    // This is for the subsystem system in Merry VM
    inlog("Starting Sub-System..."); 
    int ret = 0;
#ifdef _USE_LINUX_
    int _rfd = strtol(argv[1], NULL, 10);
    int _wfd = strtol(argv[2], NULL, 10);
    mptr_t handle;
#endif
    inlog("Sub-System waiting for confirmation...");
    read(_rfd, &ret, 1); // Wait for the subsystem to say that "You are ready to start"
    msize_t id = strtol(argv[3], NULL, 10);
    mstr_t subsysname = argv[4];
    handle = merry_loadLib(subsysname);
    if (handle == RET_NULL)
    {
        merry_subsys_send_back(_wfd, _SUBSYS_FAILED, id);
        return RET_FAILURE;
    }
    subsys_t subsys_main = merry_libsym(handle, "subsys_main");
    if (subsys_main == RET_NULL)
    {
        merry_unloadLib(&handle);
        merry_subsys_send_back(_wfd, _SUBSYS_FAILED, id);
        return RET_FAILURE;
    }
    log("Sub-System %s'%s'%s starting execution[Transferring Control]", BOLDWHITE, subsysname, RESET);
    ret = subsys_main(_rfd, _wfd, id);
    merry_unloadLib(&handle);
    close(_rfd);
    close(_wfd);
    log("Sub-System %s'%s'%s terminating...", BOLDWHITE, subsysname, RESET);
    return ret;
}