# Problems due to current implementation

Due to the way that merry is implemented right now, here are some problems that might be encountered:
1. Due to Dynamic library loading: Since dynamic library loading is performed by the Manager, it blocks handling any other service interrupts while loading the library. If a malicious library function is called, it may never return, leading to the VM being stuck in a permanent busy state. This situation lacks built-in handling mechanisms within the VM.
One potential solution is to use thread pools and assign a thread to handle the function call. This approach isolates the stalled thread instead of the entire VM. However, it introduces its own set of problems. For instance, if the VM terminates before the function call ends, data corruption could occur. Alternatively, adding a new instruction similar to DYNCALL could address the issue, but it introduces a new method for stalling the VCore execution.


# NOTES:
For the problem (1):
1. Avoid writing functions that take a long time to execute, as they can stall the VM and consume valuable execution time. Instead, upon receiving a call, start a new detached thread to perform the task while returning control. However, this approach poses challenges for library writers. The called function could dereference a pointer to obtain a "service number" indicating the requested service.
2. Load only trusted libraries to mitigate potential security risks.