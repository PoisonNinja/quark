#include <arch/cpu/cpu.h>
#include <arch/cpu/feature.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/idt.h>
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
}
}
}
