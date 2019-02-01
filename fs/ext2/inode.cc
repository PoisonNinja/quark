#include <fs/ext2/ext2.h>

namespace filesystem
{
namespace ext2
{
void ext2_instance::read_inode(ino_t ino, ext2_real_inode* out)
{
    size_t block_table_index =
        (ino - 1) / this->sb.inodes_per_group %
        (this->geometry.block_size / sizeof(ext2_bg_descriptor));

    size_t inode_block_offset =
        (((ino - 1) % this->sb.inodes_per_group) * this->sb.inode_size) /
        this->geometry.block_size;

    uint8_t buffer[this->geometry.block_size];

    read_block(buffer, this->bg_table[block_table_index].inode_table +
                           inode_block_offset);

    size_t inode_data_offset =
        ((ino - 1) % this->sb.inodes_per_group) %
        (this->geometry.block_size / sizeof(ext2_real_inode)) *
        sizeof(ext2_real_inode);

    libcxx::memcpy(out, buffer + inode_data_offset, sizeof(ext2_real_inode));
}

ext2_base_inode::ext2_base_inode(ino_t ino, ext2_instance* parent,
                                 ext2_real_inode real_inode)
    : disk_inode(real_inode)
    , instance(parent)
{
    // Copy over the information
    this->ino  = ino;
    this->mode = this->disk_inode.mode;
    this->size = this->disk_inode.size;
    this->uid  = this->disk_inode.uid;
    this->gid  = this->disk_inode.gid;
}

void ext2_base_inode::read_data_block(unsigned block_number, uint8_t* buffer)
{
    size_t real_block = 0;
    // Resolve the on-disk block #
    if (block_number < 12) {
        // No indirection needed
        real_block = this->disk_inode.block[block_number];
    } else if (block_number <
               12 + ((this->instance->geometry.block_size) / 4)) {
        log::printk(log::log_level::WARNING,
                    "ext2: Level one indirection not supported yet\n");
        // Level-one indirection
    } else {
        // TODO: Implement level-two and level-three indirection
        log::printk(log::log_level::WARNING,
                    "ext2: Level two/three indirection not supported yet\n");
        return;
    }
    this->instance->read_block(buffer, real_block);
}
} // namespace ext2
} // namespace filesystem