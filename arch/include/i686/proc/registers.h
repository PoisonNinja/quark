#pragma once

#include <types.h>

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
    uint32_t cs;
    uint32_t esp;
    uint32_t ss;
};

typedef uint32_t greg_t;
#define NGREG 16
typedef greg_t gregset_t[NGREG];

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
