const std = @import("std");

pub fn Queue(comptime Type: type) type {
    return struct {
        const Self = @This();
        var length: u64 = 0;

        const Node = struct {
            value: Type,
            next: ?*Node = null,
        };

        var head: ?*Node = null;
        var tail: ?*Node = null;
        var gpa = std.heap.GeneralPurposeAllocator(.{}){};
        var allocator: *std.mem.Allocator = gpa.allocator;

        pub fn init() Self {
            return Self{};
        }

        pub fn add_to_queue(self: *Self, value: Type) !void {
            const new_node = try self.allocator.create(Node);
            new_node.value = value;
            new_node.next = null;

            if (self.head == null) {
                // If the queue is empty, the new node becomes the head
                self.head = new_node;
                self.tail = new_node;
            } else {
                // Find the tail of the queue and append the new node
                self.tail.?.next = new_node;
                self.tail = new_node;
            }

            self.length += 1;
        }

        pub fn get_from_queue(self: *Self) ?Type {
            if (self.length == 0) {
                return null; // Return null if the queue is empty
            }

            const node_to_remove = self.head orelse return null;
            self.head = node_to_remove.next; // Move head to the next node
            self.length -= 1;

            const value = node_to_remove.value;

            // Free the removed node
            self.allocator.destroy(node_to_remove);

            return value;
        }

        pub fn deinit(self: *Self) void {
            // Free all the nodes in the queue
            while (self.head != null) {
                const next_node = self.head.?.next;
                self.allocator.destroy(self.head.?);
                self.head = next_node;
            }
            self.gpa.deinit();
        }
    };
}
