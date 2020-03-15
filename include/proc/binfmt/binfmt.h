#pragma once

#include <fs/descriptor.h>
#include <lib/list.h>
#include <proc/thread.h>
#include <types.h>

namespace binfmt
{
struct binprm {
    libcxx::intrusive_ptr<filesystem::descriptor> file;
    int argc;
    const char** argv;
    int envc;
    const char** envp;
    struct thread_context& ctx;
};

class binfmt
{
public:
    virtual const char* name() = 0;
    virtual bool
    is_match(libcxx::intrusive_ptr<filesystem::descriptor> file) = 0;
    virtual int load(struct binprm& prm)                         = 0;

    libcxx::node<binfmt> node;
};

void add(binfmt& fmt);
binfmt* get(libcxx::intrusive_ptr<filesystem::descriptor> file);
} // namespace binfmt
