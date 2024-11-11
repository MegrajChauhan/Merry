const std = @import("std");
const context = @import("include/context.zig");

pub fn main() !void {
    var cont = context.Context().init(std.heap.page_allocator);
    try cont.event_loop();
    try std.io.getStdOut().writer().print("{s}", .{"Hello World!\n"});
    return;
}
