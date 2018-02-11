#include <errno.h>
#include <fs/inode.h>
#include <lib/hashmap.h>
#include <lib/murmur.h>

namespace Filesystem
{
Inode::Inode()
{
}

Inode::~Inode()
{
}

Inode* Inode::create(Dentry* dentry, int flags, mode_t mode)
{
    return nullptr;
}

int Inode::lookup(Dentry*)
{
    return -ENOSYS;
}

namespace InodeCache
{
struct InodeKey {
    InodeKey(Inode* inode) : superblock(inode->superblock), ino(inode->ino){};
    InodeKey(Superblock* superblock, ino_t ino)
        : superblock(superblock), ino(ino){};
    Superblock* superblock;
    ino_t ino;
    bool operator==(const InodeKey& b)
    {
        return superblock == b.superblock && ino == b.ino;
    }
    bool operator!=(const InodeKey& b)
    {
        return !(*this == b);
    }
};

template <size_t hashmap_size>
struct InodeHash {
    size_t operator()(const InodeKey& key) const
    {
        uint64_t t[2] = {reinterpret_cast<uint64_t>(key.superblock), key.ino};
        uint32_t hash = Murmur::hash(t, 16);  // 8 bytes * 2 = 16 bytes
        return hash % hashmap_size;
    }
};

static Hashmap<InodeKey, Inode*, 16, InodeHash<16>>* inode_cache = nullptr;

void init()
{
    inode_cache = new Hashmap<InodeKey, Inode*, 16, InodeHash<16>>;
    Inode test;
    InodeKey key(&test);
    inode_cache->put(key, &test);
}

Inode* get(Superblock* superblock, ino_t ino)
{
    InodeKey key(superblock, ino);
    Inode* ret = nullptr;
    if (inode_cache->get(key, ret)) {
        return ret;
    } else {
        return nullptr;
    }
}

void set(Inode* inode)
{
    InodeKey key(inode);
    inode_cache->put(key, inode);
}
}
}
