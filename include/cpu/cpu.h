#pragma once

#include <arch/cpu/cpu.h>
#include <types.h>

typedef uint32_t cpu_id_t;
typedef uint8_t cpu_status_t;

namespace CPU
{
class CPU
{
public:
    cpu_id_t id;
    cpu_status_t status;
};
}
