#include <fs/descriptor.h>
#include <kernel.h>
#include <lib/string.h>

namespace Filesystem
{
Descriptor::Descriptor(Ref<Vnode> vnode)
{
    this->vnode = vnode;
}

Ref<Descriptor> Descriptor::open(const char* name, int flags, mode_t mode)
{
    char* path = String::strdup(name);
    char* current;
    Ref<Descriptor> ret(this);
    while ((current = String::strtok_r(path, "/", &path))) {
        Log::printk(Log::INFO, "%s\n", current);
        Ref<Vnode> next_vnode = vnode->open(current, flags, mode);
        if (!next_vnode) {
            return Ref<Descriptor>(nullptr);
        }
        Ref<Descriptor> next_descriptor(new Descriptor(next_vnode));
        if (!next_descriptor) {
            return Ref<Descriptor>(nullptr);
        }
        ret = next_descriptor;
    }
    delete[] path;
    return ret;
}
}
