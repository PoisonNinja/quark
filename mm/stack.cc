#include <arch/mm/layout.h>
#include <arch/mm/virtual.h>
#include <kernel.h>
#include <mm/page.h>
#include <mm/physical.h>
#include <mm/stack.h>
#include <mm/virtual.h>

namespace memory
{
void stack::push(addr_t address)
{
    scoped_lock<spinlock> _lock(this->lock);
    struct memory::page* pg = memory::pagedb::get(address);
    if (this->top) {
        top->prev = pg;
    }
    pg->next  = this->top;
    pg->prev  = nullptr;
    this->top = pg;
    this->size++;
}

void stack::remove(addr_t address)
{
    scoped_lock<spinlock> _lock(this->lock);
    struct memory::page* pg = memory::pagedb::get(address);
    if (pg->next) {
        pg->next->prev = pg->prev;
    }
    if (pg->prev) {
        pg->prev->next = pg->next;
    }
    if (pg == this->top) {
        this->top = pg->next;
    }
    this->size--;
}

addr_t stack::pop()
{
    scoped_lock<spinlock> _lock(this->lock);
    if (!this->size) {
        kernel::panic("Attempted to pop empty stack\n");
    }
    addr_t ret = memory::pagedb::addr(this->top);
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
} // namespace memory
