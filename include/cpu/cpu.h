#pragma once

#include <arch/cpu/cpu.h>
#include <proc/thread.h>
#include <types.h>

namespace CPU
{
class Core
{
public:
    uint32_t id;
    Thread* idle;

    // CPU info
    uint32_t stepping;
    uint32_t type;
    uint32_t family;
    uint32_t model;
    char vendor[13];
    char name[48];
    uint32_t features[18];
};

Core* get_current_core();
void add_core(Core* core);
}
