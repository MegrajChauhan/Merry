;; MIT License
;;
;; Copyright (c) 2024 MegrajChauhan
;;
;; Permission is hereby granted, free of charge, to any person obtaining a copy
;; of this software and associated documentation files (the "Software"), to deal
;; in the Software without restriction, including without limitation the rights
;; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;; copies of the Software, and to permit persons to whom the Software is
;; furnished to do so, subject to the following conditions:
;;
;; The above copyright notice and this permission notice shall be included in all
;; copies or substantial portions of the Software.
;;
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;; SOFTWARE.

;; Some Syscalls here use different ABIs.
;; Refer to the official linux kernel github repository for more details.

dc _M_SYS_READ_                       0        ;; Syscall number for read
dc _M_SYS_WRITE_                      1        ;; Syscall number for write
dc _M_SYS_OPEN_                       2        ;; Syscall number for open
dc _M_SYS_CLOSE_                      3        ;; Syscall number for close
dc _M_SYS_STAT_                       4        ;; Syscall number for stat
dc _M_SYS_FSTAT_                      5        ;; Syscall number for fstat
dc _M_SYS_LSTAT_                      6        ;; Syscall number for lstat
dc _M_SYS_POLL_                       7        ;; Syscall number for poll
dc _M_SYS_LSEEK_                      8        ;; Syscall number for lseek
dc _M_SYS_MMAP_                       9        ;; Syscall number for mmap
dc _M_SYS_MPROTECT_                   10       ;; Syscall number for mprotect
dc _M_SYS_MUNMAP_                     11       ;; Syscall number for munmap
dc _M_SYS_BRK_                        12       ;; Syscall number for brk(don't use this)
dc _M_SYS_RT_SIGACTION_               13       ;; Syscall number for rt_sigaction
dc _M_SYS_RT_SIGPROCMASK_             14       ;; Syscall number for rt_sigprocmask
dc _M_SYS_RT_SIGRETURN_               15       ;; Syscall number for rt_sigreturn
dc _M_SYS_IOCTL_                      16       ;; Syscall number for ioctl
dc _M_SYS_PREAD64_                    17       ;; Syscall number for pread64
dc _M_SYS_PWRITE64_                   18       ;; Syscall number for pwrite64
dc _M_SYS_READV_                      19       ;; Syscall number for readv
dc _M_SYS_WRITEV_                     20       ;; Syscall number for writev
dc _M_SYS_ACCESS_                     21       ;; Syscall number for access
dc _M_SYS_PIPE_                       22       ;; Syscall number for pipe
dc _M_SYS_SELECT_                     23       ;; Syscall number for select
dc _M_SYS_SCHED_YIELD_                24       ;; Syscall number for sched_yield
dc _M_SYS_MREMAP_                     25       ;; Syscall number for mremap
dc _M_SYS_MSYNC_                      26       ;; Syscall number for msync
dc _M_SYS_MINCORE_                    27       ;; Syscall number for mincore
dc _M_SYS_MADVISE_                    28       ;; Syscall number for madvise
dc _M_SYS_SHMGET_                     29       ;; Syscall number for shmget
dc _M_SYS_SHMAT_                      30       ;; Syscall number for shmat
dc _M_SYS_SHMCTL_                     31       ;; Syscall number for shmctl
dc _M_SYS_DUP_                        32       ;; Syscall number for dup
dc _M_SYS_DUP2_                       33       ;; Syscall number for dup2
dc _M_SYS_PAUSE_                      34       ;; Syscall number for pause
dc _M_SYS_NANOSLEEP_                  35       ;; Syscall number for nanosleep
dc _M_SYS_GETITIMER_                  36       ;; Syscall number for getitimer
dc _M_SYS_ALARM_                      37       ;; Syscall number for alarm
dc _M_SYS_SETITIMER_                  38       ;; Syscall number for setitimer
dc _M_SYS_GETPID_                     39       ;; Syscall number for getpid
dc _M_SYS_SENDFILE_                   40       ;; Syscall number for sendfile
dc _M_SYS_SOCKET_                     41       ;; Syscall number for socket
dc _M_SYS_CONNECT_                    42       ;; Syscall number for connect
dc _M_SYS_ACCEPT_                     43       ;; Syscall number for accept
dc _M_SYS_SENDTO_                     44       ;; Syscall number for sendto
dc _M_SYS_RECVFROM_                   45       ;; Syscall number for recvfrom
dc _M_SYS_SENDMSG_                    46       ;; Syscall number for sendmsg
dc _M_SYS_RECVMSG_                    47       ;; Syscall number for recvmsg
dc _M_SYS_SHUTDOWN_                   48       ;; Syscall number for shutdown
dc _M_SYS_BIND_                       49       ;; Syscall number for bind
dc _M_SYS_LISTEN_                     50       ;; Syscall number for listen
dc _M_SYS_GETSOCKNAME_                51       ;; Syscall number for getsockname
dc _M_SYS_GETPEERNAME_                52       ;; Syscall number for getpeername
dc _M_SYS_SOCKETPAIR_                 53       ;; Syscall number for socketpair
dc _M_SYS_SETSOCKOPT_                 54       ;; Syscall number for setsockopt
dc _M_SYS_GETSOCKOPT_                 55       ;; Syscall number for getsockopt
dc _M_SYS_CLONE_                      56       ;; Syscall number for clone
dc _M_SYS_FORK_                       57       ;; Syscall number for fork(don't use this)
dc _M_SYS_VFORK_                      58       ;; Syscall number for vfork
dc _M_SYS_EXECVE_                     59       ;; Syscall number for execve
dc _M_SYS_EXIT_                       60       ;; Syscall number for exit
dc _M_SYS_WAIT4_                      61       ;; Syscall number for wait4
dc _M_SYS_KILL_                       62       ;; Syscall number for kill
dc _M_SYS_UNAME_                      63       ;; Syscall number for uname
dc _M_SYS_SEMGET_                     64       ;; Syscall number for semget
dc _M_SYS_SEMOP_                      65       ;; Syscall number for semop
dc _M_SYS_SEMCTL_                     66       ;; Syscall number for semctl
dc _M_SYS_SHMDT_                      67       ;; Syscall number for shmdt
dc _M_SYS_MSGGET_                     68       ;; Syscall number for msgget
dc _M_SYS_MSGSND_                     69       ;; Syscall number for msgsnd
dc _M_SYS_MSGRCV_                     70       ;; Syscall number for msgrcv
dc _M_SYS_MSGCTL_                     71       ;; Syscall number for msgctl
dc _M_SYS_FCNTL_                      72       ;; Syscall number for fcntl
dc _M_SYS_FLOCK_                      73       ;; Syscall number for flock
dc _M_SYS_FSYNC_                      74       ;; Syscall number for fsync
dc _M_SYS_FDATASYNC_                  75       ;; Syscall number for fdatasync
dc _M_SYS_TRUNCATE_                   76       ;; Syscall number for truncate
dc _M_SYS_FTRUNCATE_                  77       ;; Syscall number for ftruncate
dc _M_SYS_GETDENTS_                   78       ;; Syscall number for getdents
dc _M_SYS_GETCWD_                     79       ;; Syscall number for getcwd
dc _M_SYS_CHDIR_                      80       ;; Syscall number for chdir
dc _M_SYS_FCHDIR_                     81       ;; Syscall number for fchdir
dc _M_SYS_RENAME_                     82       ;; Syscall number for rename
dc _M_SYS_MKDIR_                      83       ;; Syscall number for mkdir
dc _M_SYS_RMDIR_                      84       ;; Syscall number for rmdir
dc _M_SYS_CREAT_                      85       ;; Syscall number for creat
dc _M_SYS_LINK_                       86       ;; Syscall number for link
dc _M_SYS_UNLINK_                     87       ;; Syscall number for unlink
dc _M_SYS_SYMLINK_                    88       ;; Syscall number for symlink
dc _M_SYS_READLINK_                   89       ;; Syscall number for readlink
dc _M_SYS_CHMOD_                      90       ;; Syscall number for chmod
dc _M_SYS_FCHMOD_                     91       ;; Syscall number for fchmod
dc _M_SYS_CHOWN_                      92       ;; Syscall number for chown
dc _M_SYS_FCHOWN_                     93       ;; Syscall number for fchown
dc _M_SYS_LCHOWN_                     94       ;; Syscall number for lchown
dc _M_SYS_UMASK_                      95       ;; Syscall number for umask
dc _M_SYS_GETTIMEOFDAY_               96       ;; Syscall number for gettimeofday
dc _M_SYS_GETRLIMIT_                  97       ;; Syscall number for getrlimit
dc _M_SYS_GETRUSAGE_                  98       ;; Syscall number for getrusage
dc _M_SYS_SYSINFO_                    99       ;; Syscall number for sysinfo
dc _M_SYS_TIMES_                      100      ;; Syscall number for times
dc _M_SYS_PTRACE_                     101      ;; Syscall number for ptrace
dc _M_SYS_GETUID_                     102      ;; Syscall number for getuid
dc _M_SYS_SYSLOG_                     103      ;; Syscall number for syslog
dc _M_SYS_GETGID_                     104      ;; Syscall number for getgid
dc _M_SYS_SETUID_                     105      ;; Syscall number for setuid
dc _M_SYS_SETGID_                     106      ;; Syscall number for setgid
dc _M_SYS_GETEUID_                    107      ;; Syscall number for geteuid
dc _M_SYS_GETEGID_                    108      ;; Syscall number for getegid
dc _M_SYS_SETPGID_                    109      ;; Syscall number for setpgid
dc _M_SYS_GETPPID_                    110      ;; Syscall number for getppid
dc _M_SYS_GETPGRP_                    111      ;; Syscall number for getpgrp
dc _M_SYS_SETSID_                     112      ;; Syscall number for setsid
dc _M_SYS_SETREUID_                   113      ;; Syscall number for setreuid
dc _M_SYS_SETREGID_                   114      ;; Syscall number for setregid
dc _M_SYS_GETGROUPS_                  115      ;; Syscall number for getgroups
dc _M_SYS_SETGROUPS_                  116      ;; Syscall number for setgroups
dc _M_SYS_SETRESUID_                  117      ;; Syscall number for setresuid
dc _M_SYS_GETRESUID_                  118      ;; Syscall number for getresuid
dc _M_SYS_SETRESGID_                  119      ;; Syscall number for setresgid
dc _M_SYS_GETRESGID_                  120      ;; Syscall number for getresgid
dc _M_SYS_GETPGID_                    121      ;; Syscall number for getpgid
dc _M_SYS_SETFSUID_                   122      ;; Syscall number for setfsuid
dc _M_SYS_SETFSGID_                   123      ;; Syscall number for setfsgid
dc _M_SYS_GETSID_                     124      ;; Syscall number for getsid
dc _M_SYS_CAPGET_                     125      ;; Syscall number for capget
dc _M_SYS_CAPSET_                     126      ;; Syscall number for capset
dc _M_SYS_RT_SIGPENDING_              127      ;; Syscall number for rt_sigpending
dc _M_SYS_RT_SIGTIMEDWAIT_            128      ;; Syscall number for rt_sigtimedwait
dc _M_SYS_RT_SIGQUEUEINFO_            129      ;; Syscall number for rt_sigqueueinfo
dc _M_SYS_RT_SIGSUSPEND_              130      ;; Syscall number for rt_sigsuspend
dc _M_SYS_SIGALTSTACK_                131      ;; Syscall number for sigaltstack
dc _M_SYS_UTIME_                      132      ;; Syscall number for utime
dc _M_SYS_MKNOD_                      133      ;; Syscall number for mknod
dc _M_SYS_USELIB_                     134      ;; Syscall number for uselib
dc _M_SYS_PERSONALITY_                135      ;; Syscall number for personality
dc _M_SYS_USTAT_                      136      ;; Syscall number for ustat
dc _M_SYS_STATFS_                     137      ;; Syscall number for statfs
dc _M_SYS_FSTATFS_                    138      ;; Syscall number for fstatfs
dc _M_SYS_SYSFS_                      139      ;; Syscall number for sysfs
dc _M_SYS_GETPRIORITY_                140      ;; Syscall number for getpriority
dc _M_SYS_SETPRIORITY_                141      ;; Syscall number for setpriority
dc _M_SYS_SCHED_SETPARAM_             142      ;; Syscall number for sched_setparam
dc _M_SYS_SCHED_GETPARAM_             143      ;; Syscall number for sched_getparam
dc _M_SYS_SCHED_SETSCHEDULER_         144      ;; Syscall number for sched_setscheduler
dc _M_SYS_SCHED_GETSCHEDULER_         145      ;; Syscall number for sched_getscheduler
dc _M_SYS_SCHED_GET_PRIORITY_MAX_     146      ;; Syscall number for sched_get_priority_max
dc _M_SYS_SCHED_GET_PRIORITY_MIN_     147      ;; Syscall number for sched_get_priority_min
dc _M_SYS_SCHED_RR_GET_INTERVAL_      148      ;; Syscall number for sched_rr_get_interval
dc _M_SYS_MLOCK_                      149      ;; Syscall number for mlock
dc _M_SYS_MUNLOCK_                    150      ;; Syscall number for munlock
dc _M_SYS_MLOCKALL_                   151      ;; Syscall number for mlockall
dc _M_SYS_MUNLOCKALL_                 152      ;; Syscall number for munlockall
dc _M_SYS_VHANGUP_                    153      ;; Syscall number for vhangup
dc _M_SYS_MODIFY_LDT_                 154      ;; Syscall number for modify_ldt
dc _M_SYS_PIVOT_ROOT_                 155      ;; Syscall number for pivot_root
dc _M_SYS__SYSCTL_                    156      ;; Syscall number for _sysctl
dc _M_SYS_PRCTL_                      157      ;; Syscall number for prctl
dc _M_SYS_ARCH_PRCTL_                 158      ;; Syscall number for arch_prctl
dc _M_SYS_ADJTIMEX_                   159      ;; Syscall number for adjtimex
dc _M_SYS_SETRLIMIT_                  160      ;; Syscall number
dc _M_SYS_CHROOT_                     161      ;; Syscall number for chroot
dc _M_SYS_SYNC_                       162      ;; Syscall number for sync
dc _M_SYS_ACCT_                       163      ;; Syscall number for acct
dc _M_SYS_SETTIMEOFDAY_               164      ;; Syscall number for settimeofday
dc _M_SYS_MOUNT_                      165      ;; Syscall number for mount
dc _M_SYS_UMOUNT2_                    166      ;; Syscall number for umount2
dc _M_SYS_SWAPON_                     167      ;; Syscall number for swapon
dc _M_SYS_SWAPOFF_                    168      ;; Syscall number for swapoff
dc _M_SYS_REBOOT_                     169      ;; Syscall number for reboot
dc _M_SYS_SETHOSTNAME_                170      ;; Syscall number for sethostname
dc _M_SYS_SETDOMAINNAME_              171      ;; Syscall number for setdomainname
dc _M_SYS_IOPL_                       172      ;; Syscall number for iopl
dc _M_SYS_IOPERM_                     173      ;; Syscall number for ioperm
dc _M_SYS_CREATE_MODULE_              174      ;; Syscall number for create_module
dc _M_SYS_INIT_MODULE_                175      ;; Syscall number for init_module
dc _M_SYS_DELETE_MODULE_              176      ;; Syscall number for delete_module
dc _M_SYS_GET_KERNEL_SYMS_            177      ;; Syscall number for get_kernel_syms
dc _M_SYS_QUERY_MODULE_               178      ;; Syscall number for query_module
dc _M_SYS_QUOTACTL_                   179      ;; Syscall number for quotactl
dc _M_SYS_NFSSERVCTL_                 180      ;; Syscall number for nfsservctl
dc _M_SYS_GETPMSG_                    181      ;; Syscall number for getpmsg
dc _M_SYS_PUTPMSG_                    182      ;; Syscall number for putpmsg