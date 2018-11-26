#pragma once

#include <types.h>

#ifdef X86_64
struct ThreadContext {
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
#else
struct ThreadContext {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t eflags;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t cs;
    uint32_t esp;
    uint32_t ss;
    uint32_t kernel_stack;
};

using greg_t    = uint32_t;
#define NGREG 16
using gregset_t = greg_t[NGREG];

enum {
    REG_GS = 0,
#define REG_GS REG_GS
    REG_FS,
#define REG_FS REG_FS
    REG_ES,
#define REG_ES REG_ES
    REG_DS,
#define REG_DS REG_DS
    REG_EDI,
#define REG_EDI REG_EDI
    REG_ESI,
#define REG_ESI REG_ESI
    REG_EBP,
#define REG_EBP REG_EBP
    REG_ESP,
#define REG_ESP REG_ESP
    REG_EBX,
#define REG_EBX REG_EBX
    REG_EDX,
#define REG_EDX REG_EDX
    REG_ECX,
#define REG_ECX REG_ECX
    REG_EAX,
#define REG_EAX REG_EAX
    REG_EIP,
#define REG_EIP REG_EIP
    REG_CS,
#define REG_CS REG_CS
    REG_EFL,
#define REG_EFL REG_EFL
    REG_SS
#define REG_SS REG_SS
};
#endif
