#include <arch/mm/virtual.h>
#include <mm/physical.h>
#include <mm/stack.h>
#include <mm/virtual.h>

void Stack::push(addr_t address)
{
    if (this->used == this->size) {
        this->expand();
    }
    this->base[this->used++] = address;
}

addr_t Stack::pop()
{
    return this->base[--this->used];
}

bool Stack::empty()
{
    return this->used == 0;
}

void Stack::expand()
{
    addr_t phys = memory::physical::allocate();
    addr_t virt = reinterpret_cast<addr_t>(this->base + this->size);
    memory::virt::map(virt, phys, PAGE_WRITABLE);
    this->size += memory::virt::PAGE_SIZE / sizeof(addr_t);
}
