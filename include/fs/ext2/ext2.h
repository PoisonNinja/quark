#pragma once

#include <fs/driver.h>

namespace filesystem
{
namespace ext2
{
class driver : public filesystem::driver
{
public:
    driver();
    ~driver();
    bool mount(superblock* sb) override;
    uint32_t flags() override;
};

struct ext2_superblock {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;

    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;

    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;

    uint16_t def_resuid;
    uint16_t def_resgid;

    /* EXT2_DYNAMIC_REV */
    uint32_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;

    uint8_t uuid[16];
    uint8_t volume_name[16];

    uint8_t last_mounted[64];

    uint32_t algo_bitmap;

    /* Performance Hints */
    uint8_t prealloc_blocks;
    uint8_t prealloc_dir_blocks;
    uint16_t _padding;

    /* Journaling Support */
    uint8_t journal_uuid[16];
    uint32_t journal_inum;
    uint32_t jounral_dev;
    uint32_t last_orphan;

    /* Directory Indexing Support */
    uint32_t hash_seed[4];
    uint8_t def_hash_version;
    uint16_t _padding_a;
    uint8_t _padding_b;

    /* Other Options */
    uint32_t default_mount_options;
    uint32_t first_meta_bg;
    uint8_t _unused[760];

} __attribute__((packed));

struct ext2_bg_descriptor {
    uint32_t block_bitmap;
    uint32_t inode_bitmap; // block no. of inode bitmap
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    uint8_t reserved[12];
} __attribute__((packed));

/*
 * The ondisk structure of an inode
 */
struct ext2_real_inode {
    uint16_t mode;
    uint16_t uid;
    uint32_t size; // file length in byte.
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t osd1;
    uint32_t block[15];
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint8_t osd2[12];
} __attribute__((packed));

/* Represents directory entry on disk. */
struct ext2_dirent {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[]; /* Actually a set of characters, at most 255 bytes */
} __attribute__((packed));

class ext2_instance;

class ext2_file : public filesystem::inode
{
public:
};

class ext2_dir : public filesystem::inode
{
public:
    ext2_dir(ino_t ino, ext2_instance* parent, ext2_real_inode real_inode);
    virtual ~ext2_dir();
    virtual libcxx::intrusive_ptr<inode> lookup(const char* name, int flags,
                                                mode_t mode) override;

private:
    ext2_real_inode disk_inode;
    ext2_instance* instance;
};

struct ext2_calculated_geometry {
    size_t block_size;
    size_t inode_size;
    size_t num_block_groups;
    size_t bg_table_start;
};

class ext2_instance
{
public:
    ssize_t read_block(uint8_t* buffer, uint32_t block_number);
    void read_inode(ino_t ino, ext2_real_inode* out);

    libcxx::intrusive_ptr<vnode> block_device;
    libcxx::intrusive_ptr<ext2_dir> root;
    ext2_superblock sb;
    ext2_calculated_geometry geometry;
    ext2_bg_descriptor* bg_table;
};

constexpr unsigned ext2_magic = 0xEF53;
} // namespace ext2
} // namespace filesystem