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
#ifndef _MERRY_COMMANDS_
#define _MERRY_COMMANDS_

#define _GET_CORE_COUNT_ 0          // The number of vcore; active and terminated included
#define _GET_OS_ID_ 1               // The OS's id
#define _GET_DATA_MEM_PAGE_COUNT_ 2 // get the data memory's page count
#define _GET_INST_MEM_PAGE_COUNT_ 3 // get the instruction memory's page count
#define _ADD_BREAKPOINT_ 4          // add a breakpoint into the memory
#define _INST_AT_ 5                 // get the contents of the instruction memory at a given address
#define _DATA_AT_ 6                 // get the contents of the data memory at a given address
#define _SP_OF_ 7                   // get the SP of some core
#define _BP_OF_ 8                   // get the BP of some core
#define _PC_OF_ 9                   // get the PC of some core
#define _REGR_OF_ 10                // get the requested register
#define _CONTINUE_CORE_ 11          // continue the core that previously hit breakpoint

// Signals
#define _NEW_CORE_ 0          // a new core was added
#define _NEW_OS_ 1            // a new OS was created
#define _ERROR_ENCOUNTERED_ 2 // an error was encountered
#define _TERMINATING_ 3       // about to exit[after this signal the connection is closed]
#define _REPLY_ 4             // a reply for some request
#define _HIT_BP_ 5            // some core hit a break point
#define _CORE_TERMINATING_ 6  // a core is executing the halt instruction
#define _ADDED_MEM_ 7         // a new data memory page was allocated

#endif