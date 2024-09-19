# The new start
1. We have a much better file structure.
2. We have a build system setup(don't laugh at my build files.)
3. Some basic things are already setup.

# New Additions:
1. The **syscall** instruction has become protected instruction, executed by the Manager.
2. Addition of the **jmp_addr** variant of the jump instruction may help in the implementation of switch-case statements.
3. A decision to become a bit slower in return for the ability to access data that covers multiple pages.
4. The VM handles a few traps now(Using Signals- only in Linux).
5. The VM has brought back the built-in ability to load libraries dynamically.
6. New requests as well.
7. Not much progress on the stdlib but added new constants and std::fio::fopen and std::fio::fclose should work properly now.