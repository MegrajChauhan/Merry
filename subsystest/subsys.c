/**
 * The subsystems should ideally be event-drive machines.
 * Internally the subsystem may do its job however it sees fit but to the VM, it should look
 * as if the subsystem is waiting for a request, fulfills it and returns the result if asked.
 * The subsystem should also return the results in the same order as the requests came in.
 * Any core waiting for the subsystem's reply will be identified using the request number.
 * If multiple cores make the same request and the subsystem replies quicker for the second one
 * then the wrong result may be forwarded to the wrong core and thus the program may suffer.
 * The subsystem should also reserve 0 and 1 request numbers for special commands.
 * Returning 0 would imply that the subsystem failed to initialize after which the VM will
 * promptly close all other subsystems and run as is or the programs may terminate.
 * Similarly 1 implies that the subsystem has done its job and would like to terminate.
 * The VM will close the pipe and clear the corresponding channel in response and so the
 * subsystem's services should start with the number 2.
 * The subsystem can define as many services as it desires(2^64 - 3 to be exact).
 * The subsystem must have its entry function as such: int subsys_main(int rfd, int wrd, msize_t id);
 * rfd and wfd are the susbsystem's side of the pipe for sending and receiving data.
 * 'id' is a special number that the subsystem must send back when using the return number of '1'.
 *
 * How the subsystem receives its requests:
 * The subsystem will be sent 40 bytes each time with 8-byte argument, there will be 5.
 * The first argument is the request number. The remaining arguments can be anything.
 * How the subsystem sends its response:
 * The subsystem should send 2 8-byte values. The first being the request number(the one sent)
 * and the second being the actual return value.
 *
 * It is worth to note that the subsystem should and must do all memory operations on its side.
 * The subsystem won't receive any pointers and any memory related arguments.
 * Say the subsystem does high-level file processing and expects file name as argument
 * but if the file name cannot fit within the 32-byte arguments parameter then there will be problems.
 * So the program in the VM may have to use pipes of its own and send a file descriptor for
 * communication. The reply could be a number that indexes something in the subsystem but is
 * useless in the VM. This isolates the VM from the subsystem and protects(at least it tries)
 * itself from harm.
 *
 * It is also worth to note that not all services are compulsory to return.
 *
 * NOTE: If the request is '0' then the VM is requesting the subsystem to terminate.
 */

#include "queue_manager.h"
#include <stdlib.h>
#include <stdio.h>

enum
{
    _REQ_ADD = 2, // just add all arguments and return the result
};

_MERRY_INTERNAL_ Queue *queue;
_MERRY_INTERNAL_ cond_t *c;

void quick_send(int wfd, msize_t req, msize_t id)
{
    msize_t req_[2];
    req_[0] = req;
    req_[1] = id;
    write(wfd, req_, 16);
}

mptr_t simple_subsys(mptr_t arg)
{
    lock_t *l = lock_init();
    MerryThread *th = merry_thread_init();
    merry_create_detached_thread(th, &queue_manager, NULL);
    while (mtrue)
    {
        QueueItem i;
        if (queue_get_request(&i) == mfalse)
            merry_cond_wait(c, l);
        else
        {
            switch (i.request)
            {
            case 0:
            {
                queue_manager_signal();
                lock_destroy(l);
                merry_thread_destroy(th);
                return NULL;
            }
            case _REQ_ADD:
            {
                mqword_t res = i.arg[0] + i.arg[1] + i.arg[2] + i.arg[3];
                queue_send_reply(i.request, res);
                break;
            }
            }
        }
    }
    return NULL;
}

int subsys_main(int rfd, int wfd, msize_t id)
{
    // initialize the queue
    queue = queue_init(10);
    if (queue == RET_NULL)
    {
        quick_send(wfd, 0, id);
        return 0;
    }
    c = cond_init();                            // i don't think this will fail
    queue_manager_init(c, queue, wfd, rfd, id); // i don't think will fail
    // i am lazy to add checks
    MerryThread *th = merry_thread_init();
    merry_create_thread(th, &simple_subsys, NULL);
    merry_thread_join(th, NULL);
    merry_thread_destroy(th);
    return 0;
}
