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
        (((ino - 1) % this->sb.inodes_per_group) /
         (this->geometry.block_size / sizeof(ext2_real_inode)));

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
    } else if (block_number < 12 + ((this->instance->geometry.block_size) /
                                    sizeof(uint32_t))) {
        // Level-one indirection
        uint8_t buffer[this->instance->geometry.block_size];
        this->instance->read_block(buffer, this->disk_inode.block[12]);
        uint32_t* inode_table = reinterpret_cast<uint32_t*>(buffer);
        real_block            = inode_table[block_number - 12];
    } else if (block_number <
               12 + ((this->instance->geometry.block_size) / sizeof(uint32_t)) +
                   ((this->instance->geometry.block_size) / sizeof(uint32_t)) *
                       ((this->instance->geometry.block_size) /
                        sizeof(uint32_t))) {
        // Level-two indirection
        uint8_t dind_block[this->instance->geometry.block_size];
        // 14th block contains double-block pointer
        this->instance->read_block(dind_block, this->disk_inode.block[13]);
        uint32_t* dind_table = reinterpret_cast<uint32_t*>(dind_block);
        size_t dind_index =
            (block_number - 12 -
             ((this->instance->geometry.block_size) / sizeof(uint32_t))) /
            ((this->instance->geometry.block_size) / sizeof(uint32_t));
        uint8_t ind_block[this->instance->geometry.block_size];
        this->instance->read_block(ind_block, dind_table[dind_index]);
        size_t ind_index =
            (block_number - 12 -
             ((this->instance->geometry.block_size) / sizeof(uint32_t))) -
            dind_index *
                ((this->instance->geometry.block_size) / sizeof(uint32_t));
        uint32_t* ind_table = reinterpret_cast<uint32_t*>(ind_block);
        real_block          = ind_table[ind_index];
    } else {
        // TODO: Implement evel-three indirection
        log::printk(log::log_level::WARNING,
                    "ext2: Level three indirection not supported yet\n");
        return;
    }
    this->instance->read_block(buffer, real_block);
} // namespace ext2
} // namespace ext2
} // namespace filesystem
