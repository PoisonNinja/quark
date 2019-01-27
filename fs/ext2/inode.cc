#include <fs/ext2/ext2.h>

namespace filesystem
{
namespace ext2
{
namespace
{
}

ext2_dir::ext2_dir(ext2_instance* parent, ino_t ino)
    : instance(parent)
{
    size_t block_table_index =
        (ino - 1) / instance->sb.inodes_per_group %
        (instance->geometry.block_size / sizeof(ext2_bg_descriptor));

    size_t inode_block_offset = (((ino - 1) % instance->sb.inodes_per_group) *
                                 instance->sb.inode_size) /
                                instance->geometry.block_size;

    uint8_t buffer[1024];

    instance->read_block(buffer,
                         instance->bg_table[block_table_index].inode_table +
                             inode_block_offset);

    size_t inode_data_offset =
        ((ino - 1) % instance->sb.inodes_per_group) %
        (instance->geometry.block_size / sizeof(ext2_real_inode)) *
        sizeof(ext2_real_inode);

    libcxx::memcpy(&this->disk_inode, buffer + inode_data_offset,
                   sizeof(ext2_real_inode));
}
} // namespace ext2
} // namespace filesystem