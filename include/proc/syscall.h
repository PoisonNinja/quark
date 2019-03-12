#pragma once

#include <types.h>

namespace syscall
{
#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_stat 4
#define SYS_fstat 5
#define SYS_poll 7
#define SYS_lseek 8
#define SYS_mmap 9
#define SYS_mprotect 10
#define SYS_munmap 11
#define SYS_sigaction 13
#define SYS_sigprocmask 14
#define SYS_sigreturn 15
#define SYS_ioctl 16
#define SYS_dup 32
#define SYS_dup2 33
#define SYS_getpid 39
#define SYS_fork 57
#define SYS_execve 59
#define SYS_exit 60
#define SYS_kill 62
#define SYS_chdir 80
#define SYS_mkdir 83
#define SYS_sigpending 127
#define SYS_sigaltstack 131
#define SYS_mknod 133
#define SYS_chroot 161
#define SYS_mount 165
#define SYS_umount 166
#define SYS_init_module 175
#define SYS_delete_module 176

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
