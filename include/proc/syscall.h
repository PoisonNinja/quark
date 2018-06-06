#pragma once

#include <types.h>

namespace Syscall
{
#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_stat 4
#define SYS_fstat 5
#define SYS_lseek 8
#define SYS_mmap 9
#define SYS_mprotect 10
#define SYS_munmap 11
#define SYS_sigaction 13
#define SYS_sigprocmask 14
#define SYS_sigreturn 15
#define SYS_getpid 39
#define SYS_fork 57
#define SYS_execve 59
#define SYS_exit 60
#define SYS_kill 62
#define SYS_sigpending 127

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

void syscall_sysret_handler(int number, uint64_t a, uint64_t b, uint64_t c,
                            uint64_t d, uint64_t e);
void init();
}  // namespace Syscall
