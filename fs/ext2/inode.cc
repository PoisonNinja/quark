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
    size_t block_group       = (ino - 1) / instance->sb.inodes_per_group;
    size_t block_group_index = block_group / (instance->geometry.block_size /
                                              sizeof(ext2_bg_descriptor));
    log::printk(log::log_level::INFO, "ext2: block table index is %d\n",
                instance->geometry.bg_table_start + block_group_index);
    uint8_t buffer[instance->geometry.block_size];
    instance->read_block(buffer,
                         instance->geometry.bg_table_start + block_group_index);

    size_t block_table_index = block_group % (instance->geometry.block_size /
                                              sizeof(ext2_bg_descriptor));

    ext2_bg_descriptor bg;

    libcxx::memcpy(
        &bg,
        reinterpret_cast<void*>(
            (buffer + (block_table_index * sizeof(ext2_bg_descriptor)))),
        sizeof(ext2_bg_descriptor));

    size_t inode_block_offset = (((ino - 1) % instance->sb.inodes_per_group) *
                                 instance->sb.inode_size) /
                                instance->geometry.block_size;

    instance->read_block(buffer, bg.inode_table + inode_block_offset);

    size_t inode_data_offset =
        ((ino - 1) % instance->sb.inodes_per_group) %
        (instance->geometry.block_size / sizeof(ext2_real_inode)) *
        sizeof(ext2_real_inode);

    libcxx::memcpy(&this->disk_inode, buffer + inode_data_offset,
                   sizeof(ext2_real_inode));
}
} // namespace ext2
} // namespace filesystem