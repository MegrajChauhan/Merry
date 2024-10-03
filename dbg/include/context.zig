const defs = @import("defs.zig");
const fs = defs.std.fs;
const vec = defs.std.ArrayList;
const ahashmap = defs.std.AutoHashMap;

const BreakPoints = struct {
    _bp: ahashmap(u64, u64),

    pub fn add(self: *BreakPoints, key: u64, value: u64) void {
        try self._bp.fetchPut(key, value);
    }

    pub fn find(self: *BreakPoints, key: u64) ?*u64 {
        try self._bp.get(key);
    }
};

const VCore = struct {
    active: bool, // is the core active?
    hit_bp: bool, // has the core hit a breakpoint?
    bp_pc: u64, // the value of pc when a bp was hit
};

const _FILE_ = struct {
    __read_commands: fs.File, // take commands(output for the VM)
    __send_requests: fs.File, // send commands(input for the VM)
};

const _OS_STATE = struct {
    breakpoints: BreakPoints, // all of the set breakpoints
    comms: _FILE_, // communicate with the Manager
    cores: vec(VCore), // all of the cores
    dmem_page_count: u64,
    imem_page_count: u64,
    core_count: u64,
    active_core_count: u64,
    active: bool, // has the Manager terminated?
    // In future, we could keep track of the system calls and subsystems too.
};

const Context = struct {
    _all_oses: vec(_OS_STATE), // all of the Manager threads we are communicating with at the moment
};
