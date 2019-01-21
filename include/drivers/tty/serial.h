#pragma once

#include <kernel/log.h>

class serial : public log_output
{
public:
    size_t write(const char *message, size_t size) override;
};
