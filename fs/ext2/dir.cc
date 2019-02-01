#include <fs/ext2/ext2.h>
#include <fs/stat.h>

namespace filesystem
{
namespace ext2
{
ext2_dir::ext2_dir(ino_t ino, ext2_instance* parent, ext2_real_inode real_inode)
    : ext2_base_inode(ino, parent, real_inode)
{
}

ext2_dir::~ext2_dir()
{
}

libcxx::intrusive_ptr<inode> ext2_dir::lookup(const char* name, int flags,
                                              mode_t mode)
{
    log::printk(log::log_level::DEBUG, "ext2: ext2_dir::lookup: %s\n", name);

    unsigned offset = 0;
    size_t name_len = libcxx::strlen(name);
    uint8_t buffer[this->instance->geometry.block_size];
    ino_t target = 0;

    while (offset < this->size) {
        this->read_data_block(offset / this->instance->geometry.block_size,
                              buffer);
        unsigned block_offset = 0;
        while (block_offset < this->instance->geometry.block_size) {
            ext2_dirent* dirent =
                reinterpret_cast<ext2_dirent*>(buffer + block_offset);
            if (!dirent->inode || dirent->name_len != name_len) {
                block_offset += dirent->rec_len;
                offset += dirent->rec_len;
                continue;
            }
            // The names are guaranteed to be the same length, so we can use
            // strncmp
            if (!libcxx::strncmp(name, dirent->name, name_len)) {
                target = dirent->inode;
                break;
            }
            block_offset += dirent->rec_len;
            offset += dirent->rec_len;
        }
        if (target) {
            // Located :)
            ext2_real_inode real_inode;
            this->instance->read_inode(target, &real_inode);
            if (S_ISREG(real_inode.mode)) {
                return libcxx::intrusive_ptr<ext2_file>(
                    new ext2_file(target, this->instance, real_inode));
            } else if (S_ISDIR(real_inode.mode)) {
                return libcxx::intrusive_ptr<ext2_dir>(
                    new ext2_dir(target, this->instance, real_inode));
            } else {
                log::printk(log::log_level::WARNING,
                            "ext2: Unsupported file type, got mode %X\n",
                            real_inode.size);
                // TODO: Is this correct?
                return libcxx::intrusive_ptr<ext2_file>(
                    new ext2_file(target, this->instance, real_inode));
            }
        }
    }
    return libcxx::intrusive_ptr<inode>(nullptr);
}
} // namespace ext2
} // namespace filesystem