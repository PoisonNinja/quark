#include <cpu/cpu.h>

namespace cpu
{
const int max_cores = 8;
core* cores[max_cores]; // Maximum # of cores

core* get_current_core()
{
    // Hardcode initial core
    return cores[0];
}

void add_core(core* core)
{
    for (int i = 0; i < max_cores; i++) {
        if (!cores[i]) {
            cores[i] = core;
            return;
        }
    }
}
} // namespace cpu
