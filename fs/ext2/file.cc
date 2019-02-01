#include <fs/ext2/ext2.h>
#include <lib/math.h>

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
    size_t processed     = 0;
    size_t current       = offset;

    uint8_t tmp[this->instance->geometry.block_size];

    while (current_block <= end_block) {
        this->instance->read_block(tmp, current_block);
        if (current >
            libcxx::round_down(current, this->instance->geometry.block_size)) {
            size_t in_block_offset =
                current - libcxx::round_down(
                              current, this->instance->geometry.block_size);
            libcxx::memcpy(buffer + processed, tmp + in_block_offset,
                           this->instance->geometry.block_size -
                               in_block_offset);
            processed += this->instance->geometry.block_size - in_block_offset;
            current += this->instance->geometry.block_size - in_block_offset;
        } else {
            libcxx::memcpy(buffer + processed, tmp,
                           this->instance->geometry.block_size);
            processed += this->instance->geometry.block_size;
            current += this->instance->geometry.block_size;
        }
        current_block++;
    }

    return processed;
}
} // namespace ext2
} // namespace filesystem