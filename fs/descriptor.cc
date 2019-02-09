#include <errno.h>
#include <fs/descriptor.h>
#include <fs/driver.h>
#include <fs/fs.h>
#include <fs/stat.h>
#include <kernel.h>
#include <lib/string.h>

namespace filesystem
{
char* dirname(const char* path)
{
    const char* slash;
    if (path == NULL || *path == '\0')
        return libcxx::strdup(".");
    slash = path + libcxx::strlen(path) - 1;
    while (*slash == '/')
        slash--;
    if (slash != path)
        slash = (const char*)libcxx::memrchr(path, '/', slash - path);
    if (slash < path) {
        return libcxx::strdup(".");
    } else if (slash == path) {
        return libcxx::strdup("/");
    } else {
        size_t diff = slash - path;
        char* ret   = new char[diff + 1];
        libcxx::memcpy(ret, path, diff);
        ret[diff] = '\0';
        return ret;
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
            // Keep on moving the terminate back until we are no longer
            // slashes
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

descriptor::descriptor(libcxx::intrusive_ptr<vnode> vnode, int flags)
{
    this->vno            = vnode;
    this->cookie         = nullptr;
    this->current_offset = 0;
    this->flags          = flags;
}

int descriptor::ioctl(unsigned long request, char* argp)
{
    return this->vno->ioctl(request, argp, this->cookie);
}

int descriptor::link(const char* name, libcxx::intrusive_ptr<descriptor> node)
{
    const char* dir       = dirname(name);
    const char* file      = basename(name);
    auto [err, directory] = this->open(dir, O_RDONLY, 0);
    if (!directory) {
        delete[] dir;
        delete[] file;
        return -ENOENT;
    }
    int ret = directory->vno->link(file, node->vno);
    delete[] dir;
    delete[] file;
    return ret;
}

off_t descriptor::lseek(off_t offset, int whence)
{
    off_t start;
    if (whence == SEEK_SET) {
        start = 0;
    } else if (whence == SEEK_CUR) {
        start = current_offset;
    } else if (whence == SEEK_END) {
        struct filesystem::stat st;
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

int descriptor::mkdir(const char* name, mode_t mode)
{
    const char* dir  = dirname(name);
    const char* file = basename(name);
    log::printk(log::log_level::DEBUG, "[descriptor->mkdir] dir: %s file: %s\n",
                dir, file);
    auto [err, directory] = this->open(dir, O_RDONLY, 0);
    if (!directory) {
        delete[] dir;
        delete[] file;
        return -ENOENT;
    }
    int ret = directory->vno->mkdir(file, mode);
    delete[] dir;
    delete[] file;
    return ret;
}

int descriptor::mknod(const char* name, mode_t mode, dev_t dev)
{
    const char* dir  = dirname(name);
    const char* file = basename(name);
    log::printk(log::log_level::DEBUG, "[descriptor->mknod] dir: %s file: %s\n",
                dir, file);
    auto [err, directory] = this->open(dir, O_RDONLY, 0);
    if (!directory) {
        delete[] dir;
        delete[] file;
        return -ENOENT;
    }
    int ret = directory->vno->mknod(file, mode, dev);
    delete[] dir;
    delete[] file;
    return ret;
}

int descriptor::mount(const char* source, const char* target, const char* type,
                      unsigned long flags)
{
    /*
     * Retrieve the driver first so we can check if the driver actually
     * wants a real block device or doesn't care (e.g. procfs)
     */
    driver* driver = drivers::get(type);
    if (!driver) {
        log::printk(log::log_level::WARNING, "Failed to locate driver for %s\n",
                    type);
        return -EINVAL;
    }
    auto [err, target_desc] = this->open(target, O_RDONLY, 0);
    if (!target_desc) {
        return -ENOENT;
    }
    auto source_result = this->open(source, O_RDONLY, 0);
    auto source_desc   = source_result.second;
    if (!source_desc && !(driver->flags() & driver_pseudo)) {
        return -ENOENT;
    }
    superblock* sb = new superblock();
    sb->path       = source;
    if (source_desc)
        sb->source = source_desc->vno;
    driver->mount(sb);
    filesystem::mount* mt = new filesystem::mount();
    mt->sb                = sb;
    target_desc->vno->mount(mt);
    return 0;
}

libcxx::pair<int, libcxx::intrusive_ptr<descriptor>>
descriptor::open(const char* name, int flags, mode_t mode)
{
    char* path = libcxx::strdup(name);
    char* current;
    char* filename = basename(name);
    libcxx::intrusive_ptr<descriptor> ret(nullptr);
    libcxx::intrusive_ptr<vnode> curr_vnode = this->vno;
    while ((current = libcxx::strtok_r(path, "/", &path))) {
        log::printk(log::log_level::DEBUG, "[descriptor->open] %s\n", current);
        int checked_flags   = oflags_to_descriptor(flags);
        mode_t checked_mode = mode;
        if (*path != '\0') {
            checked_flags = F_READ;
            mode          = 0;
        }
        curr_vnode = curr_vnode->lookup(current, checked_flags, checked_mode);
        if (!curr_vnode) {
            log::printk(log::log_level::ERROR,
                        "[descriptor->open] Failed to open %s\n", current);
            return libcxx::make_pair(
                -ENOENT, libcxx::intrusive_ptr<descriptor>(nullptr));
        }
    }
    ret = libcxx::intrusive_ptr<descriptor>(
        new descriptor(curr_vnode, oflags_to_descriptor(flags)));
    auto [status, _cookie] = curr_vnode->open(name);
    if (!status) {
        ret->cookie = _cookie;
        log::printk(log::log_level::DEBUG,
                    "[descriptor->open] Setting cookie to %p\n", ret->cookie);
    }
    delete[] path;
    return libcxx::make_pair(0, ret);
}

ssize_t descriptor::pread(uint8_t* buffer, size_t count, off_t offset)
{
    if (!(this->flags & F_READ)) {
        log::printk(log::log_level::WARNING,
                    "Program tried to read without declaring F_READ\n");
        return -EBADF;
    }
    return vno->read(buffer, count, offset, this->cookie);
}

ssize_t descriptor::pwrite(const uint8_t* buffer, size_t count, off_t offset)
{
    if (!(this->flags & F_WRITE)) {
        log::printk(log::log_level::WARNING,
                    "Program tried to read without declaring F_READ\n");
        return -EBADF;
    }
    return vno->write(buffer, count, offset, this->cookie);
}

bool descriptor::seekable()
{
    return this->vno->seekable();
}

ssize_t descriptor::read(uint8_t* buffer, size_t count)
{
    if (!(this->flags & F_READ)) {
        log::printk(log::log_level::WARNING,
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

int descriptor::stat(struct stat* st)
{
    return vno->stat(st);
}

ssize_t descriptor::write(const uint8_t* buffer, size_t count)
{
    if (!(this->flags & F_WRITE)) {
        log::printk(log::log_level::WARNING,
                    "Program tried to read without declaring F_READ\n");
        return -EBADF;
    }
    ssize_t ret = this->vno->write(buffer, count, current_offset, this->cookie);
    if (ret > 0 && this->seekable()) {
        // TODO: Properly handle overflows
        current_offset += ret;
    }
    return ret;
}
} // namespace filesystem
