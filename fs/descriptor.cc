#include <errno.h>
#include <fs/descriptor.h>
#include <fs/fs.h>
#include <fs/ftable.h>
#include <fs/stat.h>
#include <kernel.h>
#include <lib/string.h>

namespace Filesystem
{
char* dirname(const char* path)
{
    const char* slash = libcxx::strrchr(path, '/');
    if (slash) {
        if (*(slash + 1) == '\0') {
            if (slash == path) {
                return libcxx::strdup("/");
            }
            slash = (const char*)libcxx::memrchr(path, '/', slash - path);
        }
        size_t diff = slash - path;
        if (!diff || !slash) {
            return libcxx::strdup(".");
        }
        char* ret = new char[diff + 1];
        libcxx::memcpy(ret, path, diff);
        ret[diff] = '\0';
        return ret;
    } else {
        return libcxx::strdup(".");
    }
}

char* basename(const char* path)
{
    // Locate the last slash
    const char* slash = libcxx::strrchr(path, '/');
    // Locate the end of the string we want to return
    const char* terminate = libcxx::strlen(path) + path;
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
                return libcxx::strdup(".");
            }
            // Set the location of the new slash
            slash = (const char*)libcxx::memrchr(path, '/', terminate - path);
        }
        // Allocate the return buffer
        char* ret = new char[terminate - slash + 1];
        // Copy from slash + 1 (to avoid /) to terminate
        libcxx::memcpy(ret, slash + 1, terminate - slash);
        // Null terminate
        ret[terminate - slash] = '\0';
        return ret;
    } else {
        // No slash, the entire path is a filename
        return libcxx::strdup(path);
    }
}

Descriptor::Descriptor(libcxx::intrusive_ptr<Vnode> vnode, int flags)
{
    this->vnode          = vnode;
    this->cookie         = nullptr;
    this->current_offset = 0;
    this->flags          = flags;
}

int Descriptor::ioctl(unsigned long request, char* argp)
{
    return this->vnode->ioctl(request, argp, this->cookie);
}

int Descriptor::link(const char* name, libcxx::intrusive_ptr<Descriptor> node)
{
    const char* dir                             = dirname(name);
    const char* file                            = basename(name);
    libcxx::intrusive_ptr<Descriptor> directory = this->open(dir, O_RDONLY, 0);
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
    } else {
        return -EINVAL;
    }
    off_t result = 0;
    if (__builtin_add_overflow(start, offset, &result)) {
        return -EOVERFLOW;
    }
    return current_offset = result;
}

int Descriptor::mkdir(const char* name, mode_t mode)
{
    const char* dir  = dirname(name);
    const char* file = basename(name);
    Log::printk(Log::LogLevel::DEBUG, "[descriptor->mkdir] dir: %s file: %s\n",
                dir, file);
    libcxx::intrusive_ptr<Descriptor> directory = this->open(dir, O_RDONLY, 0);
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

int Descriptor::mknod(const char* name, mode_t mode, dev_t dev)
{
    const char* dir  = dirname(name);
    const char* file = basename(name);
    Log::printk(Log::LogLevel::DEBUG, "[descriptor->mknod] dir: %s file: %s\n",
                dir, file);
    libcxx::intrusive_ptr<Descriptor> directory = this->open(dir, O_RDONLY, 0);
    if (!directory) {
        delete[] dir;
        delete[] file;
        return -ENOENT;
    }
    int ret = directory->vnode->mknod(file, mode, dev);
    delete[] dir;
    delete[] file;
    return ret;
}

int Descriptor::mount(const char* source, const char* target, const char* type,
                      unsigned long flags)
{
    /*
     * Retrieve the driver first so we can check if the driver actually wants
     * a real block device or doesn't care (e.g. procfs)
     */
    Driver* driver = Drivers::get(type);
    if (!driver) {
        Log::printk(Log::LogLevel::WARNING, "Failed to locate driver for %s\n",
                    type);
        return -EINVAL;
    }
    libcxx::intrusive_ptr<Descriptor> target_desc =
        this->open(target, O_RDONLY, 0);
    if (!target_desc) {
        return -ENOENT;
    }
    libcxx::intrusive_ptr<Descriptor> source_desc =
        this->open(source, O_RDONLY, 0);
    if (!source_desc && !(driver->flags() & driver_pseudo)) {
        return -ENOENT;
    }
    Superblock* sb = new Superblock();
    sb->path       = source;
    if (source_desc)
        sb->source = source_desc->vnode;
    driver->mount(sb);
    Mount* mt = new Mount();
    mt->sb    = sb;
    target_desc->vnode->mount(mt);
    return 0;
}

libcxx::intrusive_ptr<Descriptor> Descriptor::open(const char* name, int flags,
                                                   mode_t mode)
{
    char* path = libcxx::strdup(name);
    char* current;
    char* filename = basename(name);
    libcxx::intrusive_ptr<Descriptor> ret(nullptr);
    libcxx::intrusive_ptr<Vnode> curr_vnode = this->vnode;
    while ((current = libcxx::strtok_r(path, "/", &path))) {
        Log::printk(Log::LogLevel::DEBUG, "[descriptor->open] %s\n", current);
        int checked_flags   = flags;
        mode_t checked_mode = mode;
        if (libcxx::strcmp(current, filename)) {
            checked_flags = O_RDONLY;
            mode          = 0;
        }
        curr_vnode = curr_vnode->lookup(current, checked_flags, checked_mode);
        if (!curr_vnode) {
            Log::printk(Log::LogLevel::ERROR,
                        "[descriptor->open] Failed to open %s\n", current);
            return libcxx::intrusive_ptr<Descriptor>(nullptr);
        }
    }
    ret = libcxx::intrusive_ptr<Descriptor>(
        new Descriptor(curr_vnode, oflags_to_descriptor(flags)));
    auto [status, _cookie] = curr_vnode->open(name);
    if (!status) {
        ret->cookie = _cookie;
        Log::printk(Log::LogLevel::DEBUG,
                    "[descriptor->open] Setting cookie to %p\n", ret->cookie);
    }
    delete[] path;
    return ret;
}

ssize_t Descriptor::pread(uint8_t* buffer, size_t count, off_t offset)
{
    if (!(this->flags & F_READ)) {
        Log::printk(Log::LogLevel::WARNING,
                    "Program tried to read without declaring F_READ\n");
        return -EBADF;
    }
    return vnode->read(buffer, count, offset, this->cookie);
}

ssize_t Descriptor::pwrite(uint8_t* buffer, size_t count, off_t offset)
{
    if (!(this->flags & F_WRITE)) {
        Log::printk(Log::LogLevel::WARNING,
                    "Program tried to read without declaring F_READ\n");
        return -EBADF;
    }
    return vnode->write(buffer, count, offset, this->cookie);
}

bool Descriptor::seekable()
{
    return this->vnode->seekable();
}

ssize_t Descriptor::read(uint8_t* buffer, size_t count)
{
    if (!(this->flags & F_READ)) {
        Log::printk(Log::LogLevel::WARNING,
                    "Program tried to read without declaring F_READ\n");
        return -EBADF;
    }
    ssize_t ret = pread(buffer, count, current_offset);
    if (ret > 0 && this->seekable()) {
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
    if (!(this->flags & F_WRITE)) {
        Log::printk(Log::LogLevel::WARNING,
                    "Program tried to read without declaring F_READ\n");
        return -EBADF;
    }
    ssize_t ret =
        this->vnode->write(buffer, count, current_offset, this->cookie);
    if (ret > 0 && this->seekable()) {
        // TODO: Properly handle overflows
        current_offset += ret;
    }
    return ret;
}
} // namespace Filesystem
