#ifndef _MERRY_SYS_DEFIN_
#define _MERRY_SYS_DEFIN_

/**
 * Instead of allowing the programs to directly talk to the operating system, Merry will implement two types of system calls and be the moderator:
 * 1) Mapped syscalls: These syscalls are actually mapped to the host OS directly i.e READ would use the read syscall but the syscall number would be the same for every platform and use the host OS system call safely
 * 2) Defined syscalls: These syscalls are custom syscalls that Merry will implement itself.
 */

enum merry_syscall_num_t
{
    _SYS_EXIT,
    _SYS_OPEN,
    _SYS_CLOSE,
    _SYS_READ,
    _SYS_WRITE,
};

#endif