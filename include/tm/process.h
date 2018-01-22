#pragma once

#include <lib/list.h>
#include <tm/thread.h>
#include <types.h>

class Process
{
public:
    pid_t pid;
    Process *parent, *children;
    List<Thread, &Thread::process_node> threads;
};