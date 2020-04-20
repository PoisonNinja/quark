#include <fs/ext2/ext2.h>
#include <fs/vnode.h>
#include <kernel/init.h>
#include <lib/math.h>

namespace filesystem
{
namespace ext2
{
driver::driver()
{
}

driver::~driver()
{
}

bool driver::mount(superblock* sb)
{
    ext2_instance* ext2_fs = new ext2_instance();
    sb->source->read(reinterpret_cast<uint8_t*>(&ext2_fs->sb),
                     sizeof(ext2_superblock), 1024);
    if (ext2_fs->sb.magic != ext2_magic) {
        log::printk(log::log_level::ERROR,
                    "ext2: Bad magic. Expected %X, got %X\n", ext2_magic,
                    ext2_fs->sb.magic);
        return false;
    }
    ext2_fs->block_device = sb->source;

    // Initialize geometry
    ext2_fs->geometry.block_size = 1024 << ext2_fs->sb.log_block_size;
    if (ext2_fs->sb.rev_level < 1) {
        ext2_fs->geometry.inode_size = 128;
    } else {
        ext2_fs->geometry.inode_size = ext2_fs->sb.inode_size;
    }

    /*
     * If block size is 1024 the block group descriptor table starts in block 1,
     * otherwise in block 0, offset 1024
     */
    if (ext2_fs->geometry.block_size == 1024) {
        ext2_fs->geometry.bg_table_start = 2;
    } else {
        ext2_fs->geometry.bg_table_start = 1;
    }
    if (libcxx::div_round_up(ext2_fs->sb.blocks_count,
                             ext2_fs->sb.blocks_per_group) !=
        libcxx::div_round_up(ext2_fs->sb.inodes_count,
                             ext2_fs->sb.inodes_per_group)) {
        log::printk(log::log_level::ERROR, "ext2: Got two different answers "
                                           "for # of block groups, aborting\n");
        return false;
    }
    ext2_fs->geometry.num_block_groups = libcxx::div_round_up(
        ext2_fs->sb.blocks_count, ext2_fs->sb.blocks_per_group);

    // Print geometry
    log::printk(log::log_level::INFO, "ext2: Version: %d.%d\n",
                ext2_fs->sb.rev_level, ext2_fs->sb.minor_rev_level);
    log::printk(log::log_level::INFO, "ext2: Block size: %d bytes\n",
                ext2_fs->geometry.block_size);
    log::printk(log::log_level::INFO, "ext2: # of block groups: %zu\n",
                ext2_fs->geometry.num_block_groups);

    // Initialize block group descriptor table
    // First calculate how many blocks we need to read
    size_t num_bgd_blocks = libcxx::div_round_up(
        ext2_fs->geometry.num_block_groups * sizeof(ext2_bg_descriptor),
        ext2_fs->geometry.block_size);
    log::printk(log::log_level::INFO, "ext2: # blocks in bg table: %zu\n",
                num_bgd_blocks);

    // Allocate memory
    uint8_t* bg_buffer =
        new uint8_t[num_bgd_blocks * ext2_fs->geometry.block_size];

    // Read in the entire data
    ext2_fs->read_block(bg_buffer, ext2_fs->geometry.bg_table_start);

    // Copy into the bg_table
    ext2_fs->bg_table =
        new ext2_bg_descriptor[ext2_fs->geometry.num_block_groups];

    libcxx::memcpy(ext2_fs->bg_table, bg_buffer,
                   ext2_fs->geometry.num_block_groups *
                       sizeof(ext2_bg_descriptor));

    // Clean up the buffer
    delete[] bg_buffer;

    // ext2 root inode is always 2
    ext2_real_inode root;
    ext2_fs->read_inode(2, &root);
    sb->root = ext2_fs->root =
        libcxx::intrusive_ptr<ext2_dir>(new ext2_dir(2, ext2_fs, root));

    return true;
}

uint32_t driver::flags()
{
    return 0;
}

ssize_t ext2_instance::read_block(uint8_t* buffer, uint32_t block_number)
{
    return this->block_device->read(buffer, this->geometry.block_size,
                                    this->geometry.block_size * block_number);
}

namespace
{
ext2::driver ext2_driver;
int init()
{
    filesystem::drivers::add("ext2", &ext2_driver);
    return 0;
}
FS_INITCALL(init);
} // namespace
} // namespace ext2
} // namespace filesystem
