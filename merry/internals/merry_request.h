#ifndef _MERRY_REQUEST_
#define _MERRY_REQUEST_

#include "../../utils/merry_config.h"
#include "../../utils/merry_types.h"
#include "../../sys/merry_thread.h"

// Define a request that the OS handles
typedef struct MerryOSRequest MerryOSRequest;

/*
 Each service that the OS provides has a Number and this request struct holds that number.
 When a core posts a request, it will have to stop execution totally and wait for it's request to be fulfilled.
 This is just not efficient as valuable time and resources are wasted. Unless we have a decoder and a way to implement Out of order execution without messing up,
 inefficiency is the key.
*/

struct MerryOSRequest
{
    msize_t request_number; // this is like the interrupt number
    MerryCond *_wait_lock;  // the requesting core's condition variable
    msize_t id;             // the core's id
};

enum
{
    _REQ_PANIC_REQOVERFLOW, /*Error: The request handler has overflown. Maybe the program is delibirately trying to cause this*/
};

typedef msize_t merrot_t;

#endif