#include <fs/descriptor.h>

namespace Filesystem
{
Descriptor::Descriptor(Ref<Vnode> vnode)
{
    this->vnode = vnode;
}

Ref<Descriptor> Descriptor::open(const char* name, int flags, mode_t mode)
{
}
}
