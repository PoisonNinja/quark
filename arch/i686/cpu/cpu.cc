#include <arch/common/cpu/feature.hature.h>
#include <arch/cpu/cpu.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/idt.h>
#include <arch/cpu/syscall.h>
#include <kernel.h>

namespace CPU
{
namespace X86
{
Core bsp;

void init()
{
    CPU::X86::detect(bsp);
    CPU::add_core(&bsp);
    CPU::X86::print(bsp);
    GDT::init();
    IDT::init();
    CPU::X86::init_syscalls();
}
}  // namespace X86

void halt()
{
    __asm__("hlt");
}
}  // namespace CPU
