#include <arch/mm/layout.h>
#include <arch/mm/virtual.h>
#include <kernel.h>
#include <mm/physical.h>
#include <mm/stack.h>
#include <mm/virtual.h>

void stack::push(addr_t address)
{
    auto elem = reinterpret_cast<stack::stack_elem*>(address + PHYS_START);
    if (this->top) {
        top->prev = elem;
    }
    elem->next = this->top;
    elem->prev = nullptr;
    this->top  = elem;
    this->size++;
}

void stack::remove(addr_t address)
{
    auto elem = reinterpret_cast<stack::stack_elem*>(address + PHYS_START);
    if (elem->next) {
        elem->next->prev = elem->prev;
    }
    if (elem->prev) {
        elem->prev->next = elem->next;
    }
    if (elem == this->top) {
        this->top = elem->next;
    }
    this->size--;
}

addr_t stack::pop()
{
    if (!this->size) {
        kernel::panic("Attempted to pop empty stack\n");
    }
    addr_t ret = reinterpret_cast<addr_t>(this->top) - PHYS_START;
    this->top  = this->top->next;
    if (this->top) {
        this->top->prev = nullptr;
    }
    this->size--;
    return ret;
}

bool stack::empty()
{
    return this->size == 0;
}
