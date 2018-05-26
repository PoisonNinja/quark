#pragma once

#include <kernel/log.h>

class Serial : public LogOutput
{
public:
    size_t write(const char *message, size_t size) override;
};
