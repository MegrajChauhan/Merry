/*
 * Request representation of the Merry VM
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
#ifndef _MERRY_REQUEST_
#define _MERRY_REQUEST_

// Define a request that the OS handles
#include <merry_config.h>
#include <merry_types.h>
#include <merry_thread.h>

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
    MerryCond *_wait_lock; // the requesting core's condition variable
    msize_t id;            // the core's id

    // for debuggers
    mbyte_t opcode;
    mbyte_t arg_id;
    maddress_t address;
};

enum
{
    // these error value ranges will change with time
    /*From 0 - 50 is for the error that the internal modules generate.*/
    /*51 - 150 for any program generated errors*/
    // any remaining for actual requests
    _REQ_REQHALT = 151, /*Halt request: Halt the core that is making the request. Also check if any other cores are online, if not stop execution.*/
    _REQ_EXIT,          /*The same as halt but instead it stops every core from running*/
    _REQ_NEWCORE,       /*Create a new core for execution. This requires the requesting core's Ma register to contain the address from which the new core should execute from*/
    _REQ_MEM,           // request for more memory
    _REQ_NEWPROCESS,    // create a new process
    _REQ_INTR,          // a request of the debuggers
    _REQ_BP,            // a request for the debugger
    _REQ_GDB_INIT,      // the debugger is initialized
};

#endif
