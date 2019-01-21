#include <arch/cpu/cpu.h>
#include <arch/cpu/feature.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/idt.h>
#include <kernel.h>

namespace cpu
{
namespace x86
{
#ifdef X86_64
extern "C" void syscall_sysret_wrapper();
#else
void init_syscalls();
#endif

core bsp;

void init()
{
    // Add the BSP to the core manager
    cpu::add_core(&bsp);
    // Perform feature detections and print them out
    cpu::x86::detect(bsp);
    cpu::x86::print(bsp);
    gdt::init();
    idt::init();
#ifdef X86_64
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
    cpu::x86::wrmsr(msr_star, star);
    // Write LSTAR (syscall entry point)
    cpu::x86::wrmsr(msr_lstar,
                    reinterpret_cast<uint64_t>(&syscall_sysret_wrapper));
    /*
     * Write FMASK. Bits set here (currently IF) are unset in RFLAGS. We don't
     * want interrupts during a syscall
     */
    cpu::x86::wrmsr(msr_fmask, 0x200);
#else
    // Install the system call handler for i686
    cpu::x86::init_syscalls();
#endif
}
} // namespace x86

void halt()
{
    __asm__("hlt");
}
} // namespace cpu
