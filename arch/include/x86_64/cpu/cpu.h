#pragma once

#include <arch/cpu/idt.h>

class CPUID
{
public:
};

class CPUTables
{
public:
    struct IDT::Descriptor* idt;
};

class CPUState
{
public:
};
