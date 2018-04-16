#include <cpu/cpu.h>

namespace CPU
{
const int max_cores = 8;
Core* cores[max_cores];  // Maximum # of cores

Core* get_current_core()
{
    // Hardcode initial core
    return cores[0];
}

void add_core(Core* core)
{
    for (int i = 0; i < max_cores; i++) {
        if (!cores[i]) {
            cores[i] = core;
            return;
        }
    }
}
}
