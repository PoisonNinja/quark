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
    size_t remaining     = count;

    uint8_t tmp[this->instance->geometry.block_size];

    while (current_block <= end_block && processed < count) {
        this->read_data_block(current_block, tmp);
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
            remaining -= this->instance->geometry.block_size - in_block_offset;
        } else {
            size_t to_copy = this->instance->geometry.block_size;
            if (remaining < this->instance->geometry.block_size) {
                to_copy = remaining;
            }
            libcxx::memcpy(buffer + processed, tmp, to_copy);
            processed += to_copy;
            current += to_copy;
            remaining -= to_copy;
        }
        current_block++;
    }
    return processed;
} // namespace ext2
} // namespace ext2
} // namespace filesystem
