pub const std = @import("std");
pub const print = std.io.getStdOut().writer();
pub const read = std.io.getStdIn().reader();
pub const log = std.io.getStdErr().writer();

// Any request that the VM sends to us
pub const IncomingReq_t = enum {
    CLOSE, // we need to close the debugger
    NEW_CORE, // there was a new core creation
    NEW_OS, // a new OS was created(prepare for a new process in case it expects child debugging)
    ERR_ENCOUNTERED, // the VM encountered an error(program's erro)
    ERR_VM, // there was something with the VM
    TERMINATING, // the VM is going to terminate
    REPLY, // a reply from the VM
    BP_HIT, // some core hit a BP
    CORE_TERMINATING, // one of the cores has terminated
    MORE_MEM, // new data memory pages were added
};

// Any request that we send to the VM
pub const MakeRequest_t = enum {
    ACTIVE, // we are now active
    CLOSED, // we are now closed(no more signals)
    GET_CORE_COUNT, // the core count(total)
    GET_ACTIVE_CORE_COUNT, // the active core count
    GET_DATA_MEM_PG_COUNT, // the number of data pages in use
    GET_INST_MEM_PG_COUNT, // the number of inst pages in use
    ADD_BP, // a new breakpoint
    I_AT, // instruction at
    D_AT, // data at
    SP_OF, // SP regr of
    BP_OF, // BP regr of
    PC_OF, // PC regr of
    REGR_OF, // one regr of
    CONTINUE_CORE, // continue the core that hit BP
    BT_OF, // Get the backtrace of some core
};

