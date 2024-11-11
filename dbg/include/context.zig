const std = @import("std");
const fs = std.fs;
const array = std.ArrayList;

pub fn ChildContext() type {
    return struct {
        const TYPE = @This();
        OUT_STREAM: fs.File,
        IN_STREAM: fs.File,

        pub fn init(INPUT: fs.File, OUTPUT: fs.File) TYPE {
            return ChildContext(){ .IN_STREAM = INPUT, .OUT_STREAM = OUTPUT };
        }

        pub fn deinit(self: *TYPE) void {
            self.IN_STREAM.close();
            self.OUT_STREAM.close();
        }
    };
}

pub fn Context() type {
    return struct {
        const Self = @This();
        child_context_list: array(ChildContext()),

        pub fn init(allocator: std.mem.Allocator) Self {
            return Context(){ .child_context_list = array(ChildContext()).init(allocator) };
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
                const command: ?[]u8 = try stdin.reader().readUntilDelimiterOrEofAlloc(allocator, '\n', 256);
                defer allocator.free(command.?);

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
                    switch (state) {
                        State.Start => {
                            if (c != ' ') {
                                state = State.ParseFlag;
                            } else {
                                i += 1;
                            }
                        },
                        State.ParseFlag => {
                            if (std.ascii.isAlphanumeric(c)) {
                                try flag_name.append(c);
                                i += 1;
                            } else if ((c == ' ') or (i == (command.?.len))) {
                                state = State.ProcessCommand;
                            }
                        },
                        State.ProcessCommand => {
                            const flag_str = flag_name.items;
                            if (std.mem.eql(u8, flag_str, "exit")) {
                                keep_running = false;
                                try stdout.writer().print("Received terminating command.?...\n", .{});
                            } else if (std.mem.eql(u8, flag_str, "help")) {
                                try stdout.writer().print("Help: Available commands are -help and -exit\n", .{});
                            } else {
                                try stdout.writer().print("Unknown command: {s}\n", .{flag_str});
                            }
                            break; // Exit loop after processing command
                        },
                    }
                    if (i == command.?.len)
                        break;
                }
            }
        }
    };
}
