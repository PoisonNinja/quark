#pragma once

#define NSIGS 32

// Same as Linux
#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGIOT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPWR 30
#define SIGSYS 31

#define SIG_ERR ((void (*)(int)) - 1)
#define SIG_DFL ((void (*)(int))0)
#define SIG_IGN ((void (*)(int))1)

typedef uint32_t sigset_t;

union sigval {       /* Data passed with notification */
    int sival_int;   /* Integer value */
    void* sival_ptr; /* Pointer value */
};

typedef struct {
    union sigval si_value;
    void* si_addr;
    pid_t si_pid;
    uid_t si_uid;
    int si_signo;
    int si_code;
    int si_errno;
    int si_status;
} siginfo_t;

struct sigaction {
    union {
        void (*sa_handler)(int);
        void (*sa_sigaction)(int, siginfo_t*, void*);
    };
    sigset_t sa_mask;
    int sa_flags;
};

struct InterruptContext;

namespace Signal
{
void handle(struct InterruptContext* ctx);

int select_signal(sigset_t* set);

// Standard functions
int sigemptyset(sigset_t* set);
int sigfillset(sigset_t* set);
int sigaddset(sigset_t* set, int signum);
int sigdelset(sigset_t* set, int signum);
int sigismember(const sigset_t* set, int signum);

// Extensions
bool sigisemptyset(sigset_t* set);
// These two functions follow x86 assembly semantics where dest is modified
bool sigandset(sigset_t* dest, sigset_t* source);
bool sigorset(sigset_t* dest, sigset_t* source);

}  // namespace Signal
