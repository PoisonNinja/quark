#include <fs/ext2/ext2.h>

namespace filesystem
{
namespace ext2
{
ext2_file::ext2_file(ino_t ino, ext2_instance* parent,
                     ext2_real_inode real_inode)
    : ext2_base_inode(ino, parent, real_inode)
{
}

ext2_file::~ext2_file()
{
}

ssize_t ext2_file::read(uint8_t* buffer, size_t count, off_t offset,
                        void* cookie)
{
    size_t start_block = offset / this->instance->geometry.block_size;
    size_t end_block   = (offset + count) / this->instance->geometry.block_size;

    size_t current_block = start_block;

    uint8_t tmp[this->instance->geometry.block_size];

    while (current_block <= end_block) {
        this->instance->read_block(tmp, current_block);
        current_block++;
    }

    return 0;
}
} // namespace ext2
} // namespace filesystem