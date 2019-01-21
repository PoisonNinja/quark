#pragma once

#include <mm/virtual.h>
#include <types.h>

class stack
{
public:
    stack(addr_t* base)
        : base(base)
        , size(0)
        , used(0){};
    void push(addr_t address);
    addr_t pop();
    bool empty();

private:
    void expand();

    addr_t* base;
    size_t size;
    size_t used;
};

constexpr size_t stack_overhead(size_t total_memory, size_t block_size)
{
    return total_memory / block_size /
           (memory::virt::PAGE_SIZE / sizeof(addr_t));
}
