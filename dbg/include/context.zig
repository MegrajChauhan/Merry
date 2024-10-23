const defs = @import("defs.zig");
const utils = @import("utils.zig");
const queue = @import("queue.zig");
const fs = defs.std.fs;
const thread = defs.std.Thread;
const array = defs.std.ArrayList;

// Architecture:
// One central thread that will manage everything.
// This central thread will take in input from the USER on what to do.
// One thread per OS will run, waiting for commands from the central thread
// Each worker thread will be a 'context' of that specific OS
// Each thread will wait for any command from the central thread and based on it, send requests to the OS.
// There will not be any kind of waiting on the worker threads other than waiting for commands.

fn ChildContext() type {
    return struct {
        const TYPE = @This();
        // THREAD: thread.Handle, // maybe we won't need this
        OUT_STREAM: fs.File,
        IN_STREAM: fs.File,

        fn init(self: TYPE, INPUT: fs.File, OUTPUT: fs.File) TYPE {
            self.IN_STREAM = INPUT;
            self.OUT_STREAM = OUTPUT;
            return self;
        }

        fn deinit(self: TYPE) void {
            self.IN_STREAM.close();
            self.OUT_STREAM.close();
        }
    };
}

fn Context() type {
    return struct {
        const Self = @This();
        child_context_list: array(@TypeOf(ChildContext())),

        fn init(self: Self) Self
        {
            self.child_context_list.init(defs.std.heap.GeneralPurposeAllocator(.{}));
            return self;
        }  

        fn deinit(self: Self) void
        {
            self.child_context_list.deinit();
        }
        
    };
}
