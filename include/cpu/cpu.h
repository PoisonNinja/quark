#pragma once

#include <arch/cpu/cpu.h>
#include <types.h>

typedef uint32_t cpu_id_t;
typedef uint8_t cpu_status_t;

class CPU
{
public:
    CPUID cpuid;
    CPUState state;
    cpu_id_t id;
    cpu_status_t status;
};
