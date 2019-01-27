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

    uint8_t buffer[1024];

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
{
    // Copy over the information
    this->ino  = ino;
    this->mode = this->disk_inode.mode;
    this->size = this->disk_inode.size;
    this->uid  = this->disk_inode.uid;
    this->gid  = this->disk_inode.gid;
}

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
}
} // namespace ext2
} // namespace filesystem