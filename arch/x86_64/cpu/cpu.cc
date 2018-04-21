#include <arch/cpu/cpu.h>
#include <arch/cpu/feature.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/idt.h>
#include <kernel.h>
#include <proc/sched.h>

namespace CPU
{
namespace X64
{
extern "C" void syscall_sysret_wrapper();

Core bsp;

void init()
{
    CPU::X64::detect(bsp);
    CPU::add_core(&bsp);
    CPU::X64::print(bsp);
    GDT::init();
    IDT::init();
    /*
     * RPL=3 CS=0x23, RPL=0 CS=0x8
     *
     * There is an interesting quirk with sysret in long mode. sysret in long
     * mode adds offset of 16 to the segment in 63:48, so we manually subtract
     * that. It also forces us to have the data descriptor for ring 3 before
     * the code descriptor
     *
     */
    uint64_t star = ((((0x20ULL | 3) - 16) << 16) | ((0x8ULL))) << 32;
    // Write STAR (segments)
    CPU::X64::wrmsr(msr_star, star);
    // Write LSTAR (syscall entry point)
    CPU::X64::wrmsr(msr_lstar,
                    reinterpret_cast<uint64_t>(&syscall_sysret_wrapper));
    /*
     * Write FMASK. Bits set here (currently IF) are unset in RFLAGS. We don't
     * want interrupts during a syscall
     */
    CPU::X64::wrmsr(msr_fmask, 0x200);
}
}
}