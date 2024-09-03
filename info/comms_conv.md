# VM Debugging and Signaling Convention

## Overview

This document describes the conventions used for debugging and signaling in the VM. It includes the definitions of various requests and signals, and provides a detailed explanation of how to handle these within the VM.

## Request Definitions

### Request Codes

|Request Code| Request Name               | Description                                        |
|------------|----------------------------|----------------------------------------------------|
|          00| `_GET_CORE_COUNT_`         | Get the number of virtual cores (active and terminated included) |
|          01| `_GET_OS_ID_`              | Get the OS's ID                                    |
|          02| `_GET_DATA_MEM_PAGE_COUNT_`| Get the data memory's page count                   |
|          03| `_GET_INST_MEM_PAGE_COUNT_`| Get the instruction memory's page count            |
|          04| `_ADD_BREAKPOINT_`         | Add a breakpoint into the memory                   |
|          05| `_INST_AT_`                | Get the contents of the instruction memory at a given address |
|          06| `_DATA_AT_`                | Get the contents of the data memory at a given address |
|          07| `_SP_OF_`                  | Get the SP (Stack Pointer) of some core            |
|          08| `_BP_OF_`                  | Get the BP (Base Pointer) of some core             |
|          09| `_PC_OF_`                  | Get the PC (Program Counter) of some core          |
|          10| `_REGR_OF_`                | Get the requested register                         |
|          11| `_CONTINUE_CORE_`          | Continue the core that previously hit a breakpoint |
|          12| `_IO_PORT_NUM`             | A new process is being created and based on the flags, the VM requests a new set of IO ports to use for debugging the child|

## Signal Definitions

### Signal Codes

| Signal Code | Signal Name                | Description                                        |
|-------------|----------------------------|----------------------------------------------------|
|           00| `_NEW_CORE_`               | A new core was added                               |
|           01| `_NEW_OS_`                 | A new OS was created                               |
|           02| `_ERROR_ENCOUNTERED_`      | An error was encountered                           |
|           03| `_TERMINATING_`            | About to exit (after this signal the connection is closed) |
|           04| `_REPLY_`                  | A reply for some request                           |
|           05| `_HIT_BP_`                 | Some core hit a breakpoint                         |
|           06| `_CORE_TERMINATING_`       | A core is executing the halt instruction           |
|           07| `_ADDED_MEM_`              | A new data memory page was allocated               |

# Meaning

## Requests
Requests are the commands that any program communicating with the VM can send. The VM tries to fulfill those requests and respond similarly. A request is 16 bytes long and contains a **_Request Code_** along with some arguments. If the request needs to be replied to, the VM will use the Signal code 04 to send a reply to the program.
The first byte of every request is the opcode. The below table describes all the requests and the arguments.

|Request Code|Format                                        |
|------------|----------------------------------------------|
|00|No arguments.                                 |
|01|No arguments.                                 |
|02|No arguments.|
|03|No arguments.|
|04|6-byte address at the last 6 bytes.|
|05|6-byte address at the last 6 bytes.|
|06|6-byte address at the last 6 bytes.|
|07|The core's id at the last byte|
|08|The core's id at the last byte|
|09|The core's id at the last byte|
|10|The core's id at the last byte. The register to access at second last byte.|
|11|The core's id at the last byte|

## Signals
After processing a request, the VM replies with a signal. The signal is of the same format. The first byte is an opcode which will help the program to decipher the reply.
Here are some signals that a debugger can receive:
|Signal Code| Format|
|-----------|-------|
|00|Notice that a new core was created. The last byte contains the new core id.|
|01|Notice that a new process was created.|
|02|Notice that an error was encountered. The last byte contains the error code; New docs for error codes soon.|
|03|Notice that the signaling VM process is terminating. Last byte contains the OS id that is terminating.|
|04|The reply for some previous request such as request 00, 01 and so on.|
|05|One of the core hit the break point. The last byte contains the core ID that hit the BP.|
|06|A core is terminating. The last byte contains the core ID that hit the BP.|
|07|More memory was allocated.|

# Communicating With the VM
The VM, upon noticing the DE flag, starts two of its components namely the **listener** and **sender**. The listener only listens to the requests from the connected program. It looks for connection on port 4048.
The sender only sends back signals and replies and it looks for connection on port 4144. This structure necessitates the connecting program to have two threads as well.
_Note: The connecting program should not send more than 20 - CORE_COUNT requests at once._
This limit is because the request queue that the Managing thread manages has a maximum size of 20 and so some room for the running cores is also necessary.

## As simple as it gets
The above described convention is the only convention you require to write a debugger for the VM but one additional thing that needs to be kept in mind is that once the connected
program receives its _TERMINATING_ signal, it should make sure to send back a signal one last time so that the VM's listener also terminates peacefully. 

That's it!