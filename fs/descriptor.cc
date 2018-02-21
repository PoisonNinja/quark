#include <fs/descriptor.h>
#include <kernel.h>
#include <lib/string.h>

namespace Filesystem
{
Descriptor::Descriptor(Ref<Vnode> vnode)
{
    this->vnode = vnode;
    current_offset = 0;
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

ssize_t Descriptor::pread(uint8_t* buffer, size_t count, off_t offset)
{
    return vnode->pread(buffer, count, offset);
}

ssize_t Descriptor::pwrite(uint8_t* buffer, size_t count, off_t offset)
{
    return vnode->pwrite(buffer, count, offset);
}

ssize_t Descriptor::read(uint8_t* buffer, size_t count)
{
    ssize_t ret = pread(buffer, count, current_offset);
    if (ret > 0) {
        // TODO: Properly handle overflows
        current_offset += ret;
    }
    return ret;
}

ssize_t Descriptor::write(uint8_t* buffer, size_t count)
{
    ssize_t ret = pwrite(buffer, count, current_offset);
    if (ret > 0) {
        // TODO: Properly handle overflows
        current_offset += ret;
    }
    return ret;
}
}
