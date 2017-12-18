#pragma once

#include <kernel/log.h>

class X86Serial : public Log::LogOutput
{
public:
    size_t write(const char *message, size_t size) override;
};
