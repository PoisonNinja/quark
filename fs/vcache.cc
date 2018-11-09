#include <fs/vcache.h>
#include <kernel.h>
#include <lib/hashmap.h>
#include <lib/murmur.h>

namespace Filesystem
{
namespace
{
constexpr size_t vcache_size = 1024;

class Key
{
public:
    Key(ino_t i, dev_t d)
        : ino(i)
        , dev(d){};
    bool operator==(const Key& k) const
    {
        return (ino == k.ino) && (dev == k.dev);
    }
    bool operator!=(const Key& k) const
    {
        return !(*this == k);
    }

private:
    ino_t ino;
    dev_t dev;
};

struct Hash {
    unsigned long operator()(const Key& key)
    {
        return Murmur::hash(&key, sizeof(key)) % vcache_size;
    }
};

libcxx::unordered_map<Key, libcxx::intrusive_ptr<Vnode>, vcache_size, Hash> vcache_hash;
} // namespace

namespace VCache
{
bool add(ino_t ino, dev_t dev, libcxx::intrusive_ptr<Vnode> vnode)
{
    libcxx::intrusive_ptr<Vnode> dummy(nullptr);
    Key key(ino, dev);
    if (vcache_hash.get(key, dummy)) {
        Log::printk(Log::LogLevel::WARNING,
                    "Vnode cache already has this vnode, discarding\n");
        return false;
    }
    vcache_hash.put(key, vnode);
    return true;
}

libcxx::intrusive_ptr<Vnode> get(ino_t ino, dev_t dev)
{
    libcxx::intrusive_ptr<Vnode> dummy = libcxx::intrusive_ptr<Vnode>(nullptr);
    Key key(ino, dev);
    if (!vcache_hash.get(key, dummy)) {
        return libcxx::intrusive_ptr<Vnode>(nullptr);
    }
    return dummy;
}
} // namespace VCache
} // namespace Filesystem
