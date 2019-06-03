#pragma once

#include <mm/virtual.h>
#include <types.h>

namespace memory
{
struct page;

class stack
{
public:
    stack()
        : size(0)
        , top(nullptr){};
    void push(addr_t address);
    void remove(addr_t address);
    addr_t pop();
    bool empty();

private:
    size_t size;
    struct memory::page* top;
};
}; // namespace memory
