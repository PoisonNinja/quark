#pragma once

#include <mm/virtual.h>
#include <types.h>

class stack
{
private:
    struct stack_elem {
        stack_elem *next, *prev;
    };

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
    stack_elem* top;
};
