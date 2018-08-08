#pragma once

#include <types.h>

class Stack
{
public:
    Stack() : base(nullptr), size(0), used(0){};
    void push(addr_t address);
    addr_t pop();
    bool empty();
    void set_base(addr_t* b);

private:
    void expand();

    addr_t* base;
    size_t size;
    size_t used;
};

constexpr size_t stack_overhead(size_t total_memory, size_t block_size);
