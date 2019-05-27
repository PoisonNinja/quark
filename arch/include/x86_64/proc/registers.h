#pragma once

#include <types.h>

struct thread_context {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rsp;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t rip;
    uint64_t rflags;
    uint64_t cs;
    uint64_t ds;
    uint64_t ss;
    uint64_t gs;
    uint64_t fs;
    uint64_t kernel_stack;
};

using greg_t = uint64_t;
#define NGREG 20
using gregset_t = greg_t[NGREG];

enum {
    REG_R8 = 0,
#define REG_R8 REG_R8
    REG_R9,
#define REG_R9 REG_R9
    REG_R10,
#define REG_R10 REG_R10
    REG_R11,
#define REG_R11 REG_R11
    REG_R12,
#define REG_R12 REG_R12
    REG_R13,
#define REG_R13 REG_R13
    REG_R14,
#define REG_R14 REG_R14
    REG_R15,
#define REG_R15 REG_R15
    REG_RDI,
#define REG_RDI REG_RDI
    REG_RSI,
#define REG_RSI REG_RSI
    REG_RBP,
#define REG_RBP REG_RBP
    REG_RBX,
#define REG_RBX REG_RBX
    REG_RDX,
#define REG_RDX REG_RDX
    REG_RAX,
#define REG_RAX REG_RAX
    REG_RCX,
#define REG_RCX REG_RCX
    REG_RSP,
#define REG_RSP REG_RSP
    REG_RIP,
#define REG_RIP REG_RIP
    REG_EFL,
#define REG_EFL REG_EFL
    REG_CSGSFS, /* Actually short cs, gs, fs, __pad0.  */
#define REG_CSGSFS REG_CSGSFS
    REG_CR2
#define REG_CR2 REG_CR2
};
