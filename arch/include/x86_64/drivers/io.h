#pragma once

#include <types.h>

static inline void outb(uint16_t port, uint8_t v)
{
    asm volatile("outb %0,%1" : : "a"(v), "dN"(port));
}
static inline uint8_t inb(uint16_t port)
{
    uint8_t v;
    asm volatile("inb %1,%0" : "=a"(v) : "dN"(port));
    return v;
}

static inline void outw(uint16_t port, uint16_t v)
{
    asm volatile("outw %0,%1" : : "a"(v), "dN"(port));
}
static inline uint16_t inw(uint16_t port)
{
    uint16_t v;
    asm volatile("inw %1,%0" : "=a"(v) : "dN"(port));
    return v;
}

static inline void outl(uint16_t port, uint32_t v)
{
    asm volatile("outl %0,%1" : : "a"(v), "dN"(port));
}
static inline uint32_t inl(uint16_t port)
{
    uint32_t v;
    asm volatile("inl %1,%0" : "=a"(v) : "dN"(port));
    return v;
}

static inline void iowait(void)
{
    asm volatile("outb %%al, $0x80" : : "a"(0));
}
