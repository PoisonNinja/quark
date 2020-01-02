#pragma once

#include <types.h>

namespace syscall
{
#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_STAT 4
#define SYS_FSTAT 5
#define SYS_POLL 7
#define SYS_LSEEK 8
#define SYS_MMAP 9
#define SYS_MPROTECT 10
#define SYS_MUNMAP 11
#define SYS_SIGACTION 13
#define SYS_SIGPROCMASK 14
#define SYS_SIGRETURN 15
#define SYS_IOCTL 16
#define SYS_DUP 32
#define SYS_DUP2 33
#define SYS_GETPID 39
#define SYS_FORK 57
#define SYS_EXECVE 59
#define SYS_EXIT 60
#define SYS_WAIT 61
#define SYS_KILL 62
#define SYS_CHDIR 80
#define SYS_MKDIR 83
#define SYS_SIGPENDING 127
#define SYS_SIGALTSTACK 131
#define SYS_MKNOD 133
#define SYS_CHROOT 161
#define SYS_MOUNT 165
#define SYS_UMOUNT 166
#define SYS_INIT_MODULE 175
#define SYS_DELETE_MODULE 176

/*
 * Certain architectures (x86_64) only support passing up to 5 arguments, while
 * mmap needs 6. Therefore, create a wrapper struct to ensure that the data is
 * passed correctly.
 */
struct mmap_wrapper {
    void* addr;
    size_t length;
    int prot;
    int flags;
    int fd;
    off_t offset;
};

void init();
} // namespace syscall
