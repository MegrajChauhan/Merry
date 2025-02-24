#ifndef _MERRY_COMMANDS_
#define _MERRY_COMMANDS_

#define _DBG_ACTIVE_ 256              // the debugger is active
#define _DBG_CLOSED_ 257              // the debugger has terminated(necessary to make the debugger server in the VM to stop)
#define _GET_CORE_COUNT_ 258          // The number of vcore; active and terminated included
#define _GET_ACTIVE_CORE_COUNT_ 259   // How many cores are active?
#define _GET_DATA_MEM_PAGE_COUNT_ 260 // get the data memory's page count
#define _GET_INST_MEM_PAGE_COUNT_ 261 // get the instruction memory's page count
#define _ADD_BREAKPOINT_ 262          // add a breakpoint into the memory
#define _INST_AT_ 263                 // get the contents of the instruction memory at a given address
#define _DATA_AT_ 264                 // get the contents of the data memory at a given address
#define _SP_OF_ 265                   // get the SP of some core
#define _BP_OF_ 266                   // get the BP of some core
#define _PC_OF_ 267                   // get the PC of some core
#define _REGR_OF_ 268                 // get the requested register
#define _CONTINUE_CORE_ 269           // continue the core that previously hit breakpoint
#define _BT_OF_ 270                   // get the call stack of some core

// Signals
#define _CLOSE_ 0                // terminate yourself! sly debugger!
#define _NEW_CORE_ 1             // a new core was added
#define _NEW_OS_ 2               // a new OS was created
#define _ERROR_ENCOUNTERED_ 3    // an error was encountered
#define _ERROR_VM_ 4             // an error was encountered(not by the program)
#define _TERMINATING_ 5          // about to exit[after this signal, expected request 257 to be received]
#define _REPLY_ 4                // a reply for some request
#define _HIT_BP_ 5               // some core hit a break point
#define _CORE_TERMINATING_ 6     // a core is executing the halt instruction
#define _ADDED_MEM_ 7            // a new data memory page was allocated

#endif