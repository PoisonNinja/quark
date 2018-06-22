#include <errno.h>
#include <fs/descriptor.h>
#include <fs/fs.h>
#include <fs/stat.h>
#include <kernel.h>
#include <lib/string.h>

namespace Filesystem
{
char* dirname(const char* path)
{
    const char* slash = String::strrchr(path, '/');
    if (slash) {
        if (*(slash + 1) == '\0') {
            if (slash == path) {
                return String::strdup("/");
            }
            slash = (const char*)String::memrchr(path, '/', slash - path);
        }
        size_t diff = slash - path;
        if (!diff || !slash) {
            return String::strdup(".");
        }
        char* ret = new char[diff + 1];
        String::memcpy(ret, path, diff);
        ret[diff] = '\0';
        return ret;
    } else {
        return String::strdup(".");
    }
}

char* basename(const char* path)
{
    // Locate the last slash
    const char* slash = String::strrchr(path, '/');
    // Locate the end of the string we want to return
    const char* terminate = String::strlen(path) + path;
    if (slash) {
        // Check if this is a trailing slash
        if (*(slash + 1) == '\0') {
            // Keep on moving the terminate back until we are no longer slashes
            while (terminate > path && *(--terminate) == '/')
                ;
            /*
             * Check if we somehow ended up reaching the front (meaning the
             * entire string was just /) and return /
             */
            if (terminate == path) {
                return String::strdup(".");
            }
            // Set the location of the new slash
            slash = (const char*)String::memrchr(path, '/', terminate - path);
        }
        // Allocate the return buffer
        char* ret = new char[terminate - slash + 1];
        // Copy from slash + 1 (to avoid /) to terminate
        String::memcpy(ret, slash + 1, terminate - slash);
        // Null terminate
        ret[terminate - slash] = '\0';
        return ret;
    } else {
        // No slash, the entire path is a filename
        return String::strdup(path);
    }
}

Descriptor::Descriptor(Ref<Vnode> vnode)
{
    this->vnode = vnode;
    this->ino = vnode->ino;
    this->dev = vnode->dev;
    this->mode = vnode->mode;
    current_offset = 0;
}

int Descriptor::link(const char* name, Ref<Descriptor> node)
{
    const char* dir = dirname(name);
    const char* file = basename(name);
    Ref<Descriptor> directory = this->open(dir, O_RDONLY, 0);
    if (!directory) {
        delete[] dir;
        delete[] file;
        return -ENOENT;
    }
    int ret = directory->vnode->link(file, node->vnode);
    delete[] dir;
    delete[] file;
    return ret;
}

off_t Descriptor::lseek(off_t offset, int whence)
{
    off_t start;
    if (whence == SEEK_SET) {
        start = 0;
    } else if (whence == SEEK_CUR) {
        start = current_offset;
    } else if (whence == SEEK_END) {
        struct Filesystem::stat st;
        stat(&st);
        start = st.st_size;
    }
    off_t result = 0;
    if (__builtin_add_overflow(start, offset, &result)) {
        return -EOVERFLOW;
    }
    return current_offset = result;
}

int Descriptor::mkdir(const char* name, mode_t mode)
{
    const char* dir = dirname(name);
    const char* file = basename(name);
    Log::printk(Log::DEBUG, "[descriptor->mkdir] dir: %s file: %s\n", dir,
                file);
    Ref<Descriptor> directory = this->open(dir, O_RDONLY, 0);
    if (!directory) {
        delete[] dir;
        delete[] file;
        return -ENOENT;
    }
    int ret = directory->vnode->mkdir(file, mode);
    delete[] dir;
    delete[] file;
    return ret;
}

int Descriptor::mount(const char* source, const char* target, const char* type,
                      unsigned long flags)
{
    Ref<Descriptor> target_desc = this->open(target, O_RDONLY, 0);
    if (!target_desc) {
        return -ENOENT;
    }
    Ref<Descriptor> source_desc = this->open(source, O_RDONLY, 0);
    if (!source_desc) {
        return -ENOENT;
    }
}

Ref<Descriptor> Descriptor::open(const char* name, int flags, mode_t mode)
{
    char* path = String::strdup(name);
    char* current;
    char* filename = basename(name);
    Ref<Descriptor> ret(this);
    while ((current = String::strtok_r(path, "/", &path))) {
        Log::printk(Log::DEBUG, "[descriptor->open] %s\n", current);
        int checked_flags = flags;
        mode_t checked_mode = mode;
        if (String::strcmp(current, filename)) {
            checked_flags = O_RDONLY;
            mode = 0;
        }
        Ref<Vnode> next_vnode =
            ret->vnode->open(current, checked_flags, checked_mode);
        if (!next_vnode) {
            Log::printk(Log::ERROR, "[descriptor->open] Failed to open %s\n",
                        current);
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

bool Descriptor::seekable()
{
    if (S_ISCHR(this->mode)) {
        return false;
    }
    return true;
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

int Descriptor::stat(struct stat* st)
{
    return vnode->stat(st);
}

ssize_t Descriptor::write(uint8_t* buffer, size_t count)
{
    ssize_t ret = 0;
    if (this->seekable()) {
        ret = pwrite(buffer, count, current_offset);
        if (ret > 0) {
            // TODO: Properly handle overflows
            current_offset += ret;
        }
    } else {
        ret = vnode->write(buffer, count);
    }
    return ret;
}
}  // namespace Filesystem
