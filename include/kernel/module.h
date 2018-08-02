#pragma once

#include <lib/list.h>

bool load_module(void* binary);
bool unload_module(const char* name);