const std = @import("std");
const fs = std.fs;
const array = std.ArrayList;
const defs = @import("defs.zig");
const mt = std.Thread;

pub fn Manager() type {
    return struct {
        const TYPE = @This();
        OUT_STREAM: fs.File,
        IN_STREAM: fs.File,
        make_request: bool,
        request_kind: defs.MakeRequest_t,
        request_arg: u64,
        request_lock: mt.Mutex,

        pub fn init(INPUT: fs.File, OUTPUT: fs.File) TYPE {
            return Manager(){ .IN_STREAM = INPUT, .OUT_STREAM = OUTPUT, .make_request = false, .request_lock = mt.Mutex{} };
        }

        pub fn make_request(self: *TYPE, kind: defs.MakeRequest_t, arg: u64) bool {
            self.request_lock.lock(); // secure
            self.request_arg = arg;
            self.request_kind = kind;
            self.make_request = true;
            self.request_lock.unlock();
        }

        pub fn deinit(self: *TYPE) void {
            self.IN_STREAM.close();
            self.OUT_STREAM.close();
        }

        pub fn event_loop(self: *TYPE) void {}
    };
}

pub fn Context() type {
    return struct {
        const Self = @This();
        child_context_list: array(Manager()),

        pub fn init(allocator: std.mem.Allocator) Self {
            return Context(){ .child_context_list = array(Manager()).init(allocator) };
        }

        pub fn deinit(self: *Self) void {
            self.child_context_list.deinit();
        }

        pub fn event_loop(self: *Self) !void {
            self.deinit();

            const stdin = std.io.getStdIn();
            const stdout = std.io.getStdOut();
            const allocator = std.heap.page_allocator;

            var keep_running = true;
            while (keep_running) {
                try stdout.writer().print(">>> ", .{});

                // Read command input from the user
                const command = try stdin.reader().readUntilDelimiterOrEofAlloc(allocator, '\n', 256);
                defer if (command != null) allocator.free(command.?);

                if (command == null) {
                    try stdout.writer().print("Error: Failed to read input.\n", .{});
                    continue;
                }

                const State = enum {
                    Start,
                    ParseFlag,
                    ProcessCommand,
                };

                var state = State.Start;
                var flag_name = std.ArrayList(u8).init(allocator);
                defer flag_name.deinit();

                var i: usize = 0;
                while (true) {
                    const c = command.?.ptr[i];
                    // try stdout.writer().print("{c}\n", .{c});
                    switch (state) {
                        State.Start => {
                            if (i >= command.?.len)
                                break;
                            if (c != ' ') {
                                // try stdout.writer().print("START\n", .{});
                                state = State.ParseFlag;
                            } else {
                                i += 1;
                            }
                        },
                        State.ParseFlag => {
                            // try stdout.writer().print("PARSE FLAG\n", .{});
                            if (std.ascii.isAlphanumeric(c) and !(i >= command.?.len)) {
                                try flag_name.append(c);
                                i += 1;
                            } else {
                                state = State.ProcessCommand;
                            }
                        },
                        State.ProcessCommand => {
                            // try stdout.writer().print("COMMAND\n", .{});
                            const flag_str = flag_name.items;
                            if (std.mem.eql(u8, flag_str, "exit")) {
                                keep_running = false;
                                try stdout.writer().print("Received terminating command.\n", .{});
                            } else if (std.mem.eql(u8, flag_str, "help")) {
                                try stdout.writer().print("Help: Available commands are -help and -exit\n", .{});
                            } else {
                                try stdout.writer().print("Unknown command: {s}\n", .{flag_str});
                            }
                            flag_name.clearAndFree(); // Reset flag name for next command
                            break;
                        },
                    }
                }
            }
        }
    };
}
